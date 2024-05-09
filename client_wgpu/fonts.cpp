#include <string.h>
#include <emscripten/fetch.h>
#include <wasm_simd128.h>

template<typename T>
T rev(T a)
{
    union
    {
        u8 bytes[sizeof(T)];
        T val;
    };
    val = a;
    for(u64 i = 0; i < (sizeof(T) >> 1); i++)
    {
        u8 temp = bytes[i];
        bytes[i] = bytes[sizeof(T) - 1llu - i];
        bytes[sizeof(T) - 1llu - i] = temp;
    }
    return val;
}

template<typename T, typename Ts>
T extractr(Ts *buf, u64 offset)
{
    u8 *bb = (u8 *)buf;
    bb = &bb[offset];
    T *add = (T *)bb;
    return rev(*add);
}

template<typename T, typename Ts>
T extract(Ts *buf, u64 offset)
{
    u8 *bb = (u8 *)buf;
    bb = &bb[offset];
    T *add = (T *)bb;
    return *add;
}
template<typename T, typename Ts>
T *extractp(Ts *buf, u64 offset)
{
    u8 *bb = (u8 *)buf;
    bb = &bb[offset];
    T *add = (T *)bb;
    return add;
}
struct fontTableRec
{
    union
    {
        u8 tag[4];
        u32 tagData;
    };
    u32 cksum;
    u32 offs;
    u32 length;
};

struct fontCmapEncodingRec
{
    u16 platform;
    u16 encoding;
    u32 subtable;
};


struct fontMapping
{
    u32 numSegs;
    u32 mask16;
    u32 *starts;
    u32 *ends;
    u32 *deltas;
    u32 *offsets;
    u32 *glyphTable;
};

struct fontGlyphBox
{
    vec_t<s16, 2> start;
    vec_t<s16, 2> end;
};

struct fontGlyph
{
    vec_t<u32, 2> *contourPts;
    u32         *contourEnds;
    u32          contourNum;
    fontGlyphBox bounds;
};

struct fontGlyphs
{
    u32 num;
    fontGlyph *glyphs;
};

struct fontInfo
{
    fontMapping mapping;
    fontGlyphs glyphs;
};

void *arenaAlloc(void *, u64 amt)
{
    return malloc(amt);
}

fontInfo fontParse(void *buf, u64 bytes)
{
    void *fontArena;
    fontInfo info{};
    u32 header = extractr<u32>(buf, 0);
    u16 tables = extractr<u16>(buf, 4);
    void * glyfTable = NULL;
    void * locaTable = NULL;
    b longLoc = false;
    for(u64 i = 0; i < tables; i++)
    {
        u64 recordOff = i * sizeof(fontTableRec) + 12;
        auto rec = extractp<fontTableRec>(buf, recordOff);
        u32 *pOffs = extractp<u32>(buf, rev(rec->offs));
        u32 len = rev(rec->length);
        u32 simdsz = (len >> 4) << 2;
        v128_t state{}; 
        u8 swiz[16];
        for(u8 i = 0; i < 16; i++)
        {
            swiz[(i & (~3)) + (3 - (i & 3))] = i;
        }
        auto swizvec = wasm_v128_load(swiz);
        for(u32 i = 0 ; i < simdsz; i+=4)
        {
            auto v = wasm_v128_load(&pOffs[i]);
            auto sv = wasm_i8x16_relaxed_swizzle(v, swizvec);
            state = wasm_i32x4_add(state, sv);
        }
        u32 result[4];
        wasm_v128_store(result, state);
        u32 finalRes = result[0] + result[1] + result[2] + result[3];
        for(u32 i = 0; i < ((len >> 2) - simdsz); i++)
        {
            finalRes += rev(pOffs[i + simdsz]);
        }
        u32 diff = len - ((len >> 2) << 2);
        u8 *arr = (u8*) pOffs;
        for(u32 i = 0; i < diff; i++)
        {
            finalRes += ((u32)arr[i + ((len >> 2) << 2)])<<((3 - i) * 8);
        }
        if(rev(rec->cksum) != finalRes)
        {
            printf("Checksum error in table \"%c%c%c%c\"\n", rec->tag[0], rec->tag[1], rec->tag[2], rec->tag[3]);
        }
        u32 prc = 0;
        switch(rev(rec->tagData))
        {
            case 'maxp':
            {
                u32 ver = extractr<u32>(arr, 0);
                u16 num = extractr<u16>(arr, 4);
                info.glyphs.num = num;
                info.glyphs.glyphs = (fontGlyph *)arenaAlloc(fontArena, sizeof(fontGlyph) * num);
                break;
            }
            case 'glyf':
            {
                glyfTable  = arr;
                break;
            }
            case 'loca':
            {
                locaTable  = arr;
                break;
            }
            case 'head':
            {
                longLoc = extractr<u16>(arr, 50) == 1;
                break;
            }
            case 'cmap':
            {
                u16 version = extractr<u16>(arr, 0);
                u16 numTables = extractr<u16>(arr, 2);
                auto *subtr = extractp<fontCmapEncodingRec>(arr, 4);
                for(u16 i = 0; i < numTables; i++)
                {
                    if(rev(subtr[i].platform) != 0 && !(rev(subtr[i].platform) == 3 && (rev(subtr[i].encoding)  == 1 || rev(subtr[i].encoding)  == 10)))
                        continue; 
                    auto subt = &arr[rev(subtr[i].subtable)];
                    u16 type = extractr<u16>(subt, 0);
                    if(type == 4)
                    {
                        u16 segCnt = extractr<u16>(subt, 6) >> 1;
                        u16 *endCodes = extractp<u16>(subt, 14);
                        u16 *startCodes = &endCodes[segCnt + 1];
                        u16 *deltas = &startCodes[segCnt];
                        u16 *offsets = &deltas[segCnt];
                        info.mapping.starts = (u32*) arenaAlloc(fontArena, sizeof(u32) * segCnt);
                        info.mapping.ends = (u32*) arenaAlloc(fontArena, sizeof(u32) * segCnt);
                        info.mapping.deltas = (u32*) arenaAlloc(fontArena, sizeof(u32) * segCnt);
                        info.mapping.offsets = (u32*) arenaAlloc(fontArena, sizeof(u32) * segCnt);
                        info.mapping.numSegs = segCnt;
                        u8 *highestSeen = (u8*) &offsets[segCnt];
                        info.mapping.mask16 = 1;
                        for(u16 i = 0; i < segCnt; i++)
                        {
                            info.mapping.starts[i] = rev(startCodes[i]);
                            info.mapping.ends[i] = rev(endCodes[i]);
                            info.mapping.deltas[i] = rev(deltas[i]);
                            info.mapping.offsets[i] = 0;
                            if(offsets[i] != 0)
                            {
                                u8 *p = ((u8*)&offsets[i]) + 2 * (rev(endCodes[i]) - rev(startCodes[i])) + rev(offsets[i]) + 2;
                                if(((usize)highestSeen) < ((usize)p) ) highestSeen = p;
                                info.mapping.offsets[i] = (p - ((u8*) &offsets[segCnt])) / 2;
                            }
                        }
                        auto num = (highestSeen - (u8*)&offsets[segCnt]) / 2;
                        info.mapping.glyphTable = (u32*) arenaAlloc(fontArena, sizeof(u32) * num);
                        for(u32 i = 0; i < num; i++)
                        {
                            u16 *glypho = &offsets[segCnt];
                            info.mapping.glyphTable[i] = rev(glypho[i]);
                        }
                        break;
                    }
                    
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    for(u64 i = 0; i < info.glyphs.num; i++)
    {
        u32 offset = longLoc ? extractr<u32>(locaTable, i * 4) : extractr<u16>(locaTable, i * 2);
        u32 offsplus = longLoc ? extractr<u32>(locaTable, i * 4 + 4) : extractr<u16>(locaTable, i * 2 + 2);
        u8 *arr = extractp<u8>(glyfTable, offset * 2);

        if(offsplus == offset) continue;
        s16 numContours = extractr<s16>(arr, 0);
        info.glyphs.glyphs[i].bounds.start = vec_t<s16, 2>{extractr<s16>(arr, 2), extractr<s16>(arr, 4)};
        info.glyphs.glyphs[i].bounds.end = vec_t<s16, 2>{extractr<s16>(arr, 6), extractr<s16>(arr, 8)};
        if (numContours == -1)
        {
            // Do compound glyfs in a second pass.
            continue;
        }
        u16 totalPts = extractr<u16>(arr, 10 + ((numContours - 1) * 2));
        u16 numInsts = extractr<u16>(arr, 10 + (numContours * 2));
        u8 *insts = &arr[12 + (numContours * 2)];
        u8 *flags = &insts[numInsts];
        for(u32 i = 0; i < totalPts; i++)
        {
            
        }

        for(u16 i = 0; i < numContours; i++)
        {
            u16 totalPts = extractr<u16>(arr, 10 + (i * 2));
        }
        
        if(i == 'E'){
            printf("offset %u %d\n", offset, numContours);
        }
    }
    return info;
}

u32 fontTranslateCode(fontInfo *f, u32 code)
{
    u32 start = 0, end = f->mapping.numSegs;
    while(start != end && start != (f->mapping.numSegs - 1))
    {
        u32 median = (start + end) >> 1;
        printf("HI %u %u %u\n", start, end, median);
        if(code < f->mapping.ends[median])
        {
            end = median;
        }
        else if(code == f->mapping.ends[median])
        {
            start = median;
            break;
        }
        else
        {
            start = median + 1;
        }
    }
    if (code > f->mapping.ends[start]) start++;
    if (start >= f->mapping.numSegs) return 0;
    if (code > f->mapping.ends[start]) return 0;
    if (code < f->mapping.starts[start]) return 0;
    u32 res = f->mapping.deltas[start] + code;
    if (f->mapping.offsets[start] != 0)
    {
        res += f->mapping.glyphTable[f->mapping.offsets[start] - f->mapping.starts[start]];
    }
    if(f->mapping.mask16) res = res & ((1u << 16) - 1u);
    return res;
}


void downloadSucceeded(emscripten_fetch_t *fetch) {
  printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
  
  auto info = fontParse((void*)fetch->data, fetch->numBytes);
  printf("%c\n", fontTranslateCode(&info, 'a'));
  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void downloadFailed(emscripten_fetch_t *fetch) {
  printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
  emscripten_fetch_close(fetch); // Also free data on failure.
}

void fontTest() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = downloadSucceeded;
  attr.onerror = downloadFailed;
  emscripten_fetch(&attr, "fonts/Roboto-Regular.ttf");
}