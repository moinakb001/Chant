function g(){l.buffer!=n.buffer&&p();return aa}function ba(){l.buffer!=n.buffer&&p();return ca}function x(){l.buffer!=n.buffer&&p();return da}function y(){l.buffer!=n.buffer&&p();return ea}function z(){l.buffer!=n.buffer&&p();return fa}function ha(){l.buffer!=n.buffer&&p();return ia}function A(){l.buffer!=n.buffer&&p();return ka}var B;B||=typeof Module !== 'undefined' ? Module : {};
var la=Object.assign({},B),oa=(a,b)=>{throw b;},pa="object"==typeof window,E="function"==typeof importScripts,qa="object"==typeof process&&"object"==typeof process.versions&&"string"==typeof process.versions.node,ra=B.$ww,_scriptDir="undefined"!=typeof document&&document.currentScript?document.currentScript.src:void 0;E?_scriptDir=self.location.href:qa&&(_scriptDir=__filename);var F="";function sa(a){return B.locateFile?B.locateFile(a,F):F+a}var ta,ua,va;
if(qa){var fs=require("fs"),wa=require("path");F=E?wa.dirname(F)+"/":__dirname+"/";ta=(b,c)=>{b=xa(b)?new URL(b):wa.normalize(b);return fs.readFileSync(b,c?void 0:"utf8")};va=b=>{b=ta(b,!0);b.buffer||(b=new Uint8Array(b));return b};ua=(b,c,d,e=!0)=>{b=xa(b)?new URL(b):wa.normalize(b);fs.readFile(b,e?void 0:"utf8",(h,f)=>{h?d(h):c(e?f.buffer:f)})};process.argv.slice(2);"undefined"!=typeof module&&(module.exports=B);process.on("uncaughtException",b=>{if(!("unwind"===b||b instanceof ya||b.context instanceof
ya))throw b;});oa=(b,c)=>{process.exitCode=b;throw c;};B.inspect=()=>"[Emscripten Module object]";let a;try{a=require("worker_threads")}catch(b){throw console.error('The "worker_threads" module is not supported in this node.js build - perhaps a newer version is needed?'),b;}global.Worker=a.Worker}else if(pa||E)E?F=self.location.href:"undefined"!=typeof document&&document.currentScript&&(F=document.currentScript.src),F=0!==F.indexOf("blob:")?F.substr(0,F.replace(/[?#].*/,"").lastIndexOf("/")+1):"",
ta=a=>{var b=new XMLHttpRequest;b.open("GET",a,!1);b.send(null);return b.responseText},E&&(va=a=>{var b=new XMLHttpRequest;b.open("GET",a,!1);b.responseType="arraybuffer";b.send(null);return new Uint8Array(b.response)}),ua=(a,b,c)=>{var d=new XMLHttpRequest;d.open("GET",a,!0);d.responseType="arraybuffer";d.onload=()=>{200==d.status||0==d.status&&d.response?b(d.response):c()};d.onerror=c;d.send(null)};var za=B.print||console.log.bind(console),I=B.printErr||console.error.bind(console);
Object.assign(B,la);la=null;B.quit&&(oa=B.quit);var Aa;B.wasmBinary&&(Aa=B.wasmBinary);"object"!=typeof WebAssembly&&Ba("no native wasm support detected");var l,Ca,Da=!1,Ea,n,aa,ca,da,ea,fa,ia,ka;function p(){var a=l.buffer;B.HEAP8=n=new Int8Array(a);B.HEAP16=ca=new Int16Array(a);B.HEAPU8=aa=new Uint8Array(a);B.HEAPU16=da=new Uint16Array(a);B.HEAP32=ea=new Int32Array(a);B.HEAPU32=fa=new Uint32Array(a);B.HEAPF32=ia=new Float32Array(a);B.HEAPF64=ka=new Float64Array(a)}var Fa=B.INITIAL_MEMORY||16777216;
if(B.wasmMemory)l=B.wasmMemory;else if(l=new WebAssembly.Memory({initial:Fa/65536,maximum:32768,shared:!0}),!(l.buffer instanceof SharedArrayBuffer))throw I("requested a shared WebAssembly.Memory but the returned buffer is not a SharedArrayBuffer, indicating that while the browser has SharedArrayBuffer it does not have WebAssembly threads support - you may need to set a flag"),qa&&I("(on node you may need: --experimental-wasm-threads --experimental-wasm-bulk-memory and/or recent version)"),Error("bad memory");
p();Fa=l.buffer.byteLength;var Ga=[],Ha=[],Ia=[],Ja=[],Ka=[];function La(){if(ra){let a=B;Ma(a.sb,a.sz);removeEventListener("message",Na);Oa=Oa.forEach(Pa);addEventListener("message",Pa)}else Qa(Ha)}function Ra(){var a=B.preRun.shift();Ga.unshift(a)}var L=0,Sa=null,Ta=null;function Ua(){L++;B.monitorRunDependencies?.(L)}function Va(){L--;B.monitorRunDependencies?.(L);if(0==L&&(null!==Sa&&(clearInterval(Sa),Sa=null),Ta)){var a=Ta;Ta=null;a()}}
function Ba(a){B.onAbort?.(a);a="Aborted("+a+")";I(a);Da=!0;Ea=1;throw new WebAssembly.RuntimeError(a+". Build with -sASSERTIONS for more info.");}var Wa=a=>a.startsWith("data:application/octet-stream;base64,"),xa=a=>a.startsWith("file://"),Xa;Xa="app.wasm";Wa(Xa)||(Xa=sa(Xa));function Ya(a){if(a==Xa&&Aa)return new Uint8Array(Aa);if(va)return va(a);throw"both async and sync fetching of the wasm failed";}
function Za(a){if(!Aa&&(pa||E)){if("function"==typeof fetch&&!xa(a))return fetch(a,{credentials:"same-origin"}).then(b=>{if(!b.ok)throw"failed to load wasm binary file at '"+a+"'";return b.arrayBuffer()}).catch(()=>Ya(a));if(ua)return new Promise((b,c)=>{ua(a,d=>b(new Uint8Array(d)),c)})}return Promise.resolve().then(()=>Ya(a))}function $a(a,b,c){return Za(a).then(d=>WebAssembly.instantiate(d,b)).then(d=>d).then(c,d=>{I(`failed to asynchronously prepare wasm: ${d}`);Ba(d)})}
function ab(a,b){var c=Xa;Aa||"function"!=typeof WebAssembly.instantiateStreaming||Wa(c)||xa(c)||qa||"function"!=typeof fetch?$a(c,a,b):fetch(c,{credentials:"same-origin"}).then(d=>WebAssembly.instantiateStreaming(d,a).then(b,function(e){I(`wasm streaming compile failed: ${e}`);I("falling back to ArrayBuffer instantiation");return $a(c,a,b)}))}
var bb={6006:()=>{document.querySelector("body").innerHTML="Please use a WebGPU-compatible browser."},6094:function(){let a=Atomics.waitAsync(y(),arguments[0]>>2,arguments[1]),b=Array.prototype.slice.call(arguments,4,arguments.length),c=arguments[2]>>2,d=arguments[3],e=0==b.length?"":"i".repeat(b.length);a.async&&a.value.then(()=>{console.log("promise finished");B.dynCall("v"+e,d,b)});y()[c]=!!a.async}};
function ya(a){this.name="ExitStatus";this.message=`Program terminated with exit(${a})`;this.status=a}var Oa=[],cb=[],db,M=a=>{var b=cb[a];b||(a>=cb.length&&(cb.length=a+1),cb[a]=b=db.get(a));return b},Pa=a=>{a=qa?a:a.data;let b=a._wsc;b&&M(b)(...a.x)},Na=a=>{Oa.push(a)},Qa=a=>{for(;0<a.length;)a.shift()(B)},eb=B.noExitRuntime||!0,fb={},gb=1,hb=[],ib=0;function jb(){for(var a=N.length-1;0<=a;--a)kb(a);N=[];lb=[]}var lb=[];
function mb(){if(navigator.userActivation?navigator.userActivation.isActive:ib&&nb.s)for(var a=0;a<lb.length;++a){var b=lb[a];lb.splice(a,1);--a;b.I.apply(null,b.G)}}var N=[];function kb(a){var b=N[a];b.target.removeEventListener(b.g,b.D,b.l);N.splice(a,1)}
function ob(a){function b(d){++ib;nb=a;mb();a.o(d);mb();--ib}if(!a.target)return-4;if(a.m)a.D=b,a.target.addEventListener(a.g,b,a.l),N.push(a),pb||(Ja.push(jb),pb=!0);else for(var c=0;c<N.length;++c)N[c].target==a.target&&N[c].g==a.g&&kb(c--);return 0}
var pb,nb,qb,rb,sb,tb="undefined"!=typeof TextDecoder?new TextDecoder("utf8"):void 0,ub=(a,b)=>{for(var c=b+NaN,d=b;a[d]&&!(d>=c);)++d;if(16<d-b&&a.buffer&&tb)return tb.decode(a.buffer instanceof SharedArrayBuffer?a.slice(b,d):a.subarray(b,d));for(c="";b<d;){var e=a[b++];if(e&128){var h=a[b++]&63;if(192==(e&224))c+=String.fromCharCode((e&31)<<6|h);else{var f=a[b++]&63;e=224==(e&240)?(e&15)<<12|h<<6|f:(e&7)<<18|h<<12|f<<6|a[b++]&63;65536>e?c+=String.fromCharCode(e):(e-=65536,c+=String.fromCharCode(55296|
e>>10,56320|e&1023))}}else c+=String.fromCharCode(e)}return c},O=a=>a?ub(g(),a):"",vb=[0,"undefined"!=typeof document?document:0,"undefined"!=typeof window?window:0],wb=a=>{a=2<a?O(a):a;return vb[a]||("undefined"!=typeof document?document.querySelector(a):void 0)},xb=a=>0>vb.indexOf(a)?a.getBoundingClientRect():{left:0,top:0},yb=(a,b,c,d)=>{qb||=P(36);a=wb(a);return ob({target:a,g:"resize",m:d,o:(e=event)=>{if(e.target==a){var h=document.body;if(h){var f=qb;y()[f>>2]=e.detail;y()[f+4>>2]=h.clientWidth;
y()[f+8>>2]=h.clientHeight;y()[f+12>>2]=innerWidth;y()[f+16>>2]=innerHeight;y()[f+20>>2]=outerWidth;y()[f+24>>2]=outerHeight;y()[f+28>>2]=pageXOffset;y()[f+32>>2]=pageYOffset;M(d)(10,f,b)&&e.preventDefault()}}},l:c})},zb=(a,b,c,d,e,h)=>{rb||=P(1696);a=wb(a);return ob({target:a,s:"touchstart"==h||"touchend"==h,g:h,m:d,o:f=>{for(var k,m={},u=f.touches,r=0;r<u.length;++r)k=u[r],k.v=k.A=0,m[k.identifier]=k;for(r=0;r<f.changedTouches.length;++r)k=f.changedTouches[r],k.v=1,m[k.identifier]=k;for(r=0;r<f.targetTouches.length;++r)m[f.targetTouches[r].identifier].A=
1;u=rb;A()[u>>3]=f.timeStamp;var t=u>>2;y()[t+3]=f.ctrlKey;y()[t+4]=f.shiftKey;y()[t+5]=f.altKey;y()[t+6]=f.metaKey;t+=7;var G=xb(a),C=0;for(r in m)if(k=m[r],y()[t]=k.identifier,y()[t+1]=k.screenX,y()[t+2]=k.screenY,y()[t+3]=k.clientX,y()[t+4]=k.clientY,y()[t+5]=k.pageX,y()[t+6]=k.pageY,y()[t+7]=k.v,y()[t+8]=k.A,y()[t+9]=k.clientX-G.left,y()[t+10]=k.clientY-G.top,t+=13,31<++C)break;y()[u+8>>2]=C;M(d)(e,u,b)&&f.preventDefault()},l:c})},Ab=(a,b,c,d)=>{sb||=P(104);return ob({target:a,s:!0,g:"wheel",
m:d,o:(e=event)=>{var h=sb;A()[h>>3]=e.timeStamp;var f=h>>2;y()[f+2]=e.screenX;y()[f+3]=e.screenY;y()[f+4]=e.clientX;y()[f+5]=e.clientY;y()[f+6]=e.ctrlKey;y()[f+7]=e.shiftKey;y()[f+8]=e.altKey;y()[f+9]=e.metaKey;ba()[2*f+20]=e.button;ba()[2*f+21]=e.buttons;y()[f+11]=e.movementX;y()[f+12]=e.movementY;var k=xb(a);y()[f+13]=e.clientX-k.left;y()[f+14]=e.clientY-k.top;A()[h+72>>3]=e.deltaX;A()[h+80>>3]=e.deltaY;A()[h+88>>3]=e.deltaZ;y()[h+96>>2]=e.deltaMode;M(d)(9,h,b)&&e.preventDefault()},l:c})};
function Bb(){this.h=[void 0];this.u=[]}var R,Cb;
function Db(a,b,c,d,e){function h(){var w=0,H=0;q.response&&D&&0===z()[a+12>>2]&&(H=q.response.byteLength);0<H&&(w=P(H),g().set(new Uint8Array(q.response),w));z()[a+12>>2]=w;T(a+16,H);T(a+24,0);(w=q.response?q.response.byteLength:0)&&T(a+32,w);x()[a+40>>1]=q.readyState;x()[a+42>>1]=q.status;q.statusText&&U(q.statusText,a+44,64)}var f=z()[a+8>>2];if(f){var k=O(f),m=a+112,u=O(m+0);u||="GET";var r=z()[m+56>>2],t=z()[m+68>>2],G=z()[m+72>>2];f=z()[m+76>>2];var C=z()[m+80>>2],Q=z()[m+84>>2],J=z()[m+88>>
2],K=z()[m+52>>2],D=!!(K&1),ja=!!(K&2);K=!!(K&64);t=t?O(t):void 0;G=G?O(G):void 0;var q=new XMLHttpRequest;q.withCredentials=!!g()[m+60>>0];q.open(u,k,!K,t,G);K||(q.timeout=r);q.i=k;q.responseType="arraybuffer";C&&(k=O(C),q.overrideMimeType(k));if(f)for(;;){m=z()[f>>2];if(!m)break;k=z()[f+4>>2];if(!k)break;f+=8;m=O(m);k=O(k);q.setRequestHeader(m,k)}var S=R.C(q);z()[a>>2]=S;f=Q&&J?g().slice(Q,Q+J):null;q.onload=w=>{R.has(S)&&(h(),200<=q.status&&300>q.status?b?.(a,q,w):c?.(a,q,w))};q.onerror=w=>{R.has(S)&&
(h(),c?.(a,q,w))};q.ontimeout=w=>{R.has(S)&&c?.(a,q,w)};q.onprogress=w=>{if(R.has(S)){var H=D&&ja&&q.response?q.response.byteLength:0,v=0;0<H&&D&&ja&&(v=P(H),g().set(new Uint8Array(q.response),v));z()[a+12>>2]=v;T(a+16,H);T(a+24,w.loaded-H);T(a+32,w.total);x()[a+40>>1]=q.readyState;3<=q.readyState&&0===q.status&&0<w.loaded&&(q.status=200);x()[a+42>>1]=q.status;q.statusText&&U(q.statusText,a+44,64);d?.(a,q,w);v&&Eb(v)}};q.onreadystatechange=w=>{R.has(S)&&(x()[a+40>>1]=q.readyState,2<=q.readyState&&
(x()[a+42>>1]=q.status),e?.(a,q,w))};try{q.send(f)}catch(w){c?.(a,q,w)}}else c(a,0,"no url specified!")}
var Fb=a=>{a instanceof ya||"unwind"==a||oa(1,a)},Gb=a=>{Ea=a;eb||(B.onExit?.(a),Da=!0);oa(a,new ya(a))},T=(a,b)=>{z()[a>>2]=b;var c=z()[a>>2];z()[a+4>>2]=(b-c)/4294967296},U=(a,b,c)=>{var d=g();if(0<c){c=b+c-1;for(var e=0;e<a.length;++e){var h=a.charCodeAt(e);if(55296<=h&&57343>=h){var f=a.charCodeAt(++e);h=65536+((h&1023)<<10)|f&1023}if(127>=h){if(b>=c)break;d[b++]=h}else{if(2047>=h){if(b+1>=c)break;d[b++]=192|h>>6}else{if(65535>=h){if(b+2>=c)break;d[b++]=224|h>>12}else{if(b+3>=c)break;d[b++]=240|
h>>18;d[b++]=128|h>>12&63}d[b++]=128|h>>6&63}d[b++]=128|h&63}}d[b]=0}};function Hb(a,b,c,d){var e=Cb;if(e){var h=a+112;(h=z()[h+64>>2])||(h=z()[a+8>>2]);var f=O(h);try{var k=e.transaction(["FILES"],"readwrite").objectStore("FILES").put(b,f);k.onsuccess=()=>{x()[a+40>>1]=4;x()[a+42>>1]=200;U("OK",a+44,64);c(a,0,f)};k.onerror=m=>{x()[a+40>>1]=4;x()[a+42>>1]=413;U("Payload Too Large",a+44,64);d(a,0,m)}}catch(m){d(a,0,m)}}else d(a,0,"IndexedDB not available!")}
function Ib(a,b,c){var d=Cb;if(d){var e=a+112;(e=z()[e+64>>2])||(e=z()[a+8>>2]);e=O(e);try{var h=d.transaction(["FILES"],"readonly").objectStore("FILES").get(e);h.onsuccess=f=>{if(f.target.result){f=f.target.result;var k=f.byteLength||f.length,m=P(k);g().set(new Uint8Array(f),m);z()[a+12>>2]=m;T(a+16,k);T(a+24,0);T(a+32,k);x()[a+40>>1]=4;x()[a+42>>1]=200;U("OK",a+44,64);b(a,0,f)}else x()[a+40>>1]=4,x()[a+42>>1]=404,U("Not Found",a+44,64),c(a,0,"no data")};h.onerror=f=>{x()[a+40>>1]=4;x()[a+42>>1]=
404;U("Not Found",a+44,64);c(a,0,f)}}catch(f){c(a,0,f)}}else c(a,0,"IndexedDB not available!")}
function Jb(a,b,c){var d=Cb;if(d){var e=a+112;(e=z()[e+64>>2])||(e=z()[a+8>>2]);e=O(e);try{var h=d.transaction(["FILES"],"readwrite").objectStore("FILES").delete(e);h.onsuccess=f=>{f=f.target.result;z()[a+12>>2]=0;T(a+16,0);T(a+24,0);T(a+32,0);x()[a+40>>1]=4;x()[a+42>>1]=200;U("OK",a+44,64);b(a,0,f)};h.onerror=f=>{x()[a+40>>1]=4;x()[a+42>>1]=404;U("Not Found",a+44,64);c(a,0,f)}}catch(f){c(a,0,f)}}else c(a,0,"IndexedDB not available!")}var Kb=[null,[],[]];
String.prototype.replaceAll||(String.prototype.replaceAll=function(a,b){"?"===a&&(a="\\?");return this.replace(a instanceof RegExp?a:new RegExp(a,"g"),b)});function V(a,b,c){c=c||65;for(b=b.split("|");b[0];)a=a.replaceAll(String.fromCharCode(c++),b.pop());return[,].concat(a.split(" "))}
var W=V("r8YA8TA8SA8UALSALUALWR8YR8TR8SR8UANSANUANWRLSRLURLW V8Y V8Z V8T V8S V8U bgra8Y bgra8ZRb9e5uWRbJa2SRbJa2YR11bJuWRNSRNURNW VLS VLU VLW VNS VNU VNWB8ILYI24plusI24plus-E8INWINW-E8Q1-V-YQ1-V-ZQ2-V-YQ2-V-ZQ3-V-YQ3-V-ZQ4-r-YQ4-r-TQ5-rg-YQ5-rg-TQ6h-rgb-uWQ6h-rgb-WQ7-V-YQ7-V-ZPYPZPa1YPa1Z etc2-V8Y etc2-V8ZFr11YFr11TFrg11YFrg11TX4x4-YX4x4-ZX5x4-YX5x4-ZX5x5-YX5x5-ZX6x5-YX6x5-ZX6x6-YX6x6-ZX8x5-YX8x5-ZX8x6-YX8x6-ZX8x8-YX8x8-ZXJx5-YXJx5-ZXJx6-YXJx6-ZXJx8-YXJx8-ZXJxJ-YXJxJ-ZX12xJ-YX12xJ-ZX12x12-YX12x12-Z S8MS8KU8MU8KY8MY8KT8MT8KSLMSLKULMULKYLMYLKTLMTLKWLMWLKWN WNMWNx3 WNKSN SNMSNx3 SNKUN UNMUNx3 UNKYJ-J-J-2","unorm-srgb|unorm| astc-|float|rgba|sint|snorm|uint| rg| bc| etc2-rgb8|-AC|32|x2 |16|x4 |10| depth|-B|SC| eac-|stencil|-ESJ|-E-A| E| r"),
X={},Lb=1;function Y(a){if(a){for(;X[++Lb];)2147483646<Lb&&(Lb=1);X[Lb]=a;return a.B=Lb}}function Mb(a){return b=>{try{return a(b)}catch(c){}}}
var Nb=V("B-clip-control B32D-Aencil8EbcEetc2EaAc timeAamp-query indirect-firA-inAance shader-f16 rg11b10uD-rendC bgra8unorm-Aorage D32-filtC"," texture-compression-|float|erable|depth|st").slice(1),Ob=V(">1D >2D >3D maxTextureArrayLayer<6<6sPlus7=<BindingsPer6 maxDynamicUniform=;DynamicS:=;SampledTexture@maxSampler@maxS:=@maxS:Texture@maxUniform=@minUniform=9inS:=9ax7=<7Attribute<7=ArrayStride max8Component<8Variable<ColorAttachment<ColorAttachmentBytesPerSample?pS:Size maxComputeInvocationsPerWorkgroup?pSizeX?pSizeY?pSizeZ","sPerShaderStage | maxComputeWorkgrou|maxTextureDimension|Buffer|s max|sPerPipelineLayout max|torage|OffsetAlignment m|InterStageShader|Vertex|BindGroup|Uniform7|8ColorAttachmen",
52).slice(1),Pb=V("maxUniform4Storage4BufferSize","BufferBindingSize max",52).slice(1);function Qb(a){return z()[a]+4294967296*z()[a+1]}var Rb=[,"srgb","display-p3"];function Sb(a,b){a>>=2;let c=[];for(;b--;)c.push(X[z()[a++]]);return c}function Tb(a){let b=X[a];b&&(b.destroy&&b.destroy(),b.j&&b.j.forEach(Tb),delete X[a])}var Ub=[,"load","clear"],Vb=[,"store","discard"];function Wb(a,b){(a=Y(a))&&b.j.push(a);return a}
var Xb=V("neverA equalACB notCBCalways","-equal |greater| less"),Yb=V("keep zero replace invert inCBdeCBinCA deCA","crement-|clamp |wrap");function Zb(a){return{compare:Xb[z()[a]],failOp:Yb[z()[a+1]],depthFailOp:Yb[z()[a+2]],passOp:Yb[z()[a+3]]}}var $b=V("add Areverse-Amin max","subtract "),ac=V("zero one BEB BDEBD AEA ADEAD BD-saturated CEC"," one-minus-|-alpha|constant|src|dst");function bc(a){return{operation:$b[z()[a]],srcFactor:ac[z()[a+1]],dstFactor:ac[z()[a+2]]}}
function cc(a,b){let c={};for(;b--;)c[O(z()[a>>2])]=A()[a+8>>3],a+=16;return c}var dc=V("A16 A32","uint"),ec=V("pointDADAB CDCB","-list |triangle|-strip|line");
function fc(a){var b=[];let c=[],d=a>>2,e=y()[d+2],h=z()[d+3]>>2,f=d+6;a=f+5;let k=a+17,m=k+3,u=y()[m+2],r=z()[m+3]>>2,t=z()[a++],G=z()[k],C=z()[m],Q=z()[m+6];for(;e--;){let J=[],K=y()[h],D=z()[h+1]>>2;for(;K--;)J.push({offset:Qb(D),shaderLocation:z()[D+2],format:W[z()[D+3]]}),D+=4;b.push({arrayStride:Qb(h+2),stepMode:[,"vertex","instance"][z()[h+4]],attributes:J});h+=6}for(;u--;)c.push(z()[r]?{format:W[z()[r]],blend:z()[r+1]?{color:bc(r+1),alpha:bc(r+4)}:void 0,writeMask:z()[r+7]}:null),r+=8;b={vertex:{module:X[z()[d]],
entryPoint:O(z()[d+1])||void 0,buffers:b,constants:cc(z()[d+5],y()[d+4])},primitive:{topology:ec[z()[f]],stripIndexFormat:dc[z()[f+1]],frontFace:[,"ccw","cw"][z()[f+2]],cullMode:[,"none","front","back"][z()[f+3]],unclippedDepth:!!z()[f+4]},layout:1<Q?X[Q]:"auto"};t&&(b.depthStencil={format:W[t],depthWriteEnabled:!!z()[a++],depthCompare:Xb[z()[a++]],stencilReadMask:z()[a++],stencilWriteMask:z()[a++],depthBias:y()[a++],depthBiasSlopeScale:ha()[a++],depthBiasClamp:ha()[a++],stencilFront:Zb(a),stencilBack:Zb(a+
4),clampDepth:!!z()[a+8]});G&&(b.multisample={count:G,mask:z()[k+1],alphaToCoverageEnabled:!!z()[k+2]});C&&(b.fragment={module:X[C],entryPoint:O(z()[m+1])||void 0,targets:c,constants:cc(z()[m+5],y()[m+4])});return b}
var gc=a=>{for(var b=0,c=0;c<a.length;++c){var d=a.charCodeAt(c);127>=d?b++:2047>=d?b+=2:55296<=d&&57343>=d?(b+=4,++c):b+=3}return b},kc=(a,b,c,d)=>{var e={string:u=>{var r=0;if(null!==u&&void 0!==u&&0!==u){r=gc(u)+1;var t=hc(r);U(u,t,r);r=t}return r},array:u=>{var r=hc(u.length);l.buffer!=n.buffer&&p();n.set(u,r);return r}};a=B["_"+a];var h=[],f=0;if(d)for(var k=0;k<d.length;k++){var m=e[c[k]];m?(0===f&&(f=ic()),h[k]=m(d[k])):h[k]=d[k]}c=a.apply(null,h);return c=function(u){0!==f&&jc(f);return"string"===
b?O(u):"boolean"===b?!!u:u}(c)};ra&&(fb[0]=this,addEventListener("message",Na));R=new Bb;Ua();(function(a,b){try{var c=indexedDB.open("emscripten_filesystem",1)}catch(d){b(d);return}c.onupgradeneeded=d=>{d=d.target.result;d.objectStoreNames.contains("FILES")&&d.deleteObjectStore("FILES");d.createObjectStore("FILES")};c.onsuccess=d=>a(d.target.result);c.onerror=b})(a=>{Cb=a;Va()},()=>{Cb=!1;Va()});
Object.assign(Bb.prototype,{get(a){return this.h[a]},has(a){return void 0!==this.h[a]},C(a){var b=this.u.pop()||this.h.length;this.h[b]=a;return b},F(a){this.h[a]=void 0;this.u.push(a)}});
var lc={_emscripten_create_wasm_worker:(a,b)=>{let c=fb[gb]=new Worker(sa("app.ww.js"));c.postMessage({$ww:gb,wasm:Ca,js:B.mainScriptUrlOrBlob||_scriptDir,wasmMemory:l,sb:a,sz:b});c.onmessage=Pa;return gb++},_emscripten_fetch_free:function(a){if(R.has(a)){var b=R.get(a);R.F(a);0<b.readyState&&4>b.readyState&&b.abort()}},emscripten_asm_const_int:(a,b,c)=>{hb.length=0;for(var d;d=g()[b++];){var e=105!=d;e&=112!=d;c+=e&&c%8?4:0;hb.push(112==d?z()[c>>2]:105==d?y()[c>>2]:A()[c>>3]);c+=e?8:4}return bb[a].apply(null,
hb)},emscripten_get_device_pixel_ratio:()=>"number"==typeof devicePixelRatio&&devicePixelRatio||1,emscripten_get_element_css_size:(a,b,c)=>{a=wb(a);if(!a)return-4;a=xb(a);A()[b>>3]=a.width;A()[c>>3]=a.height;return 0},emscripten_get_now:()=>performance.now(),emscripten_is_main_browser_thread:()=>!E,emscripten_performance_now:()=>performance.now(),emscripten_request_animation_frame_loop:(a,b)=>{function c(d){M(a)(d,b)&&requestAnimationFrame(c)}return requestAnimationFrame(c)},emscripten_resize_heap:a=>
{var b=g().length;a>>>=0;if(a<=b||2147483648<a)return!1;for(var c=1;4>=c;c*=2){var d=b*(1+.2/c);d=Math.min(d,a+100663296);var e=Math;d=Math.max(a,d);a:{e=(e.min.call(e,2147483648,d+(65536-d%65536)%65536)-l.buffer.byteLength+65535)/65536;try{l.grow(e);p();var h=1;break a}catch(f){}h=void 0}if(h)return!0}return!1},emscripten_set_resize_callback_on_thread:(a,b,c,d)=>yb(a,b,c,d),emscripten_set_touchcancel_callback_on_thread:(a,b,c,d)=>zb(a,b,c,d,25,"touchcancel"),emscripten_set_touchend_callback_on_thread:(a,
b,c,d)=>zb(a,b,c,d,23,"touchend"),emscripten_set_touchmove_callback_on_thread:(a,b,c,d)=>zb(a,b,c,d,24,"touchmove"),emscripten_set_touchstart_callback_on_thread:(a,b,c,d)=>zb(a,b,c,d,22,"touchstart"),emscripten_set_wheel_callback_on_thread:(a,b,c,d)=>(a=wb(a))?"undefined"!=typeof a.onwheel?Ab(a,b,c,d):-1:-4,emscripten_start_fetch:function(a,b,c,d,e){function h(v){if(G)v();else if(!Da)try{if(v(),!eb)try{Ea=v=Ea,Gb(v)}catch(ma){Fb(ma)}}catch(ma){Fb(ma)}}var f=a+112,k=z()[f+36>>2],m=z()[f+40>>2],u=z()[f+
44>>2],r=z()[f+48>>2],t=z()[f+52>>2],G=!!(t&64),C=v=>{h(()=>{k?M(k)(v):b?.(v)})},Q=v=>{h(()=>{u?M(u)(v):d?.(v)})},J=v=>{h(()=>{m?M(m)(v):c?.(v)})},K=v=>{h(()=>{r?M(r)(v):e?.(v)})},D=v=>{Db(v,C,J,Q,K)},ja=(v,ma)=>{Hb(v,ma.response,na=>{h(()=>{k?M(k)(na):b?.(na)})},na=>{h(()=>{k?M(k)(na):b?.(na)})})},q=v=>{Db(v,ja,J,Q,K)},S=O(f+0),w=!!(t&16),H=!!(t&4);t=!!(t&32);if("EM_IDB_STORE"===S)D=z()[f+84>>2],f=z()[f+88>>2],Hb(a,g().slice(D,D+f),C,J);else if("EM_IDB_DELETE"===S)Jb(a,C,J);else if(w){if(t)return 0;
Db(a,H?ja:C,J,Q,K)}else Ib(a,C,t?J:H?q:D);return a},emscripten_wasm_worker_post_function_v:(a,b)=>{fb[a].postMessage({_wsc:b,x:[]})},fd_write:(a,b,c,d)=>{for(var e=0,h=0;h<c;h++){var f=z()[b>>2],k=z()[b+4>>2];b+=8;for(var m=0;m<k;m++){var u=g()[f+m],r=Kb[a];0===u||10===u?((1===a?za:I)(ub(r,0)),r.length=0):r.push(u)}e+=k}z()[d>>2]=e;return 0},memory:l,navigator_gpu_available:function(){return!!navigator.gpu},navigator_gpu_get_preferred_canvas_format:function(){return W.indexOf(navigator.gpu.getPreferredCanvasFormat())},
navigator_gpu_request_adapter_async:function(a,b,c){a>>=2;let d=navigator.gpu,e=[,"low-power","high-performance"][z()[a]],h={};if(d){a&&(h.forceFallbackAdapter=!!z()[a+1],e&&(h.powerPreference=e));function f(k){M(b)(Y(k),c)}d.requestAdapter(h).then(Mb(f)).catch(()=>{f()});return 1}},postCanvas:function(a,b){b=document.querySelector(O(b)).transferControlToOffscreen();fb[a].postMessage({canvas:b},[b])},renderHijackMessage:function(a){addEventListener("message",b=>{b=b.data;"canvas"in b&&(self.i=b.canvas,
console.log(self.i),B._renderAdapterInit(a))})},setOffscreenSize:function(a,b){self.i.width=a;self.i.height=b},wgpu_adapter_request_device_async:function(a,b,c,d){function e(m){m&&(m.j=[],Y(m.queue));M(c)(Y(m),d)}b>>=2;let h=[],f={},k=z()[b];b+=2;for(let m=0;11>m;++m)k&1<<m&&h.push(Nb[m]);for(let m of Pb){if(k=Qb(b))f[m]=k;b+=2}for(let m of Ob)if(k=z()[b++])f[m]=k;b=z()[b];X[a].requestDevice({requiredFeatures:h,requiredLimits:f,defaultQueue:b?{label:O(b)}:void 0}).then(Mb(e)).catch(()=>{e()})},wgpu_canvas_context_configure:function(a,
b){b>>=2;X[a].configure({device:X[z()[b]],format:W[z()[b+1]],usage:z()[b+2],viewFormats:Sb(z()[b+4],z()[b+3]),colorSpace:Rb[z()[b+5]],alphaMode:[,"opaque","premultiplied"][z()[b+6]]})},wgpu_canvas_context_get_current_texture:function(a){a=X[a].getCurrentTexture();a!=X[1]&&(Tb(1),X[1]=a,a.B=1,a.j=[]);return 1},wgpu_canvas_get_webgpu_context:function(){return Y(self.i.getContext("webgpu"))},wgpu_command_encoder_begin_render_pass:function(a,b){b>>=2;let c=[];for(var d=y()[b++],e=z()[b++]>>2,h=e+6>>1,
f=X[z()[b]];d--;)c.push(z()[e]?{view:X[z()[e]],depthSlice:0>y()[e+1]?void 0:y()[e+1],resolveTarget:X[z()[e+2]],storeOp:Vb[z()[e+3]],loadOp:Ub[z()[e+4]],clearValue:[A()[h],A()[h+1],A()[h+2],A()[h+3]]}:null),e+=14,h+=7;a=X[a];d=a.beginRenderPass;f=f?{view:f,depthLoadOp:Ub[z()[b+1]],depthClearValue:ha()[b+2],depthStoreOp:Vb[z()[b+3]],depthReadOnly:!!z()[b+4],stencilLoadOp:Ub[z()[b+5]],stencilClearValue:z()[b+6],stencilStoreOp:Vb[z()[b+7]],stencilReadOnly:!!z()[b+8]}:void 0;e=X[z()[b+9]];h=A()[b+10>>
1]||void 0;var k;b+=12;var m=z()[b];m?(m={querySet:X[m]},0<=(k=y()[b+1])&&(m.beginningOfPassWriteIndex=k),0<=(k=y()[b+2])&&(m.endOfPassWriteIndex=k),k=m):k=void 0;return Y(d.call(a,{colorAttachments:c,depthStencilAttachment:f,occlusionQuerySet:e,maxDrawCount:h,timestampWrites:k}))},wgpu_device_create_bind_group:function(a,b,c,d){a=X[a];c>>=2;let e=[];for(;d--;){var h=z()[c],f=X[z()[c+1]];h={binding:h,resource:f};if(f.isBuffer){f={buffer:f,offset:Qb(c+2)};let k=Qb(c+4);k&&(f.size=k);h.resource=f}e.push(h);
c+=6}return Wb(a.createBindGroup({layout:X[b],entries:e}),a)},wgpu_device_create_buffer:function(a,b){a=X[a];b>>=2;b=a.createBuffer({size:Qb(b),usage:z()[b+2],mappedAtCreation:!!z()[b+3]});b.H={};b.isBuffer=1;return Wb(b,a)},wgpu_device_create_command_encoder:function(a){return Wb(X[a].createCommandEncoder(void 0),X[a])},wgpu_device_create_render_bundle_encoder:function(a,b){a=X[a];b>>=2;let c=[],d=y()[b],e=z()[b+1]>>2;for(;d--;)c.push(W[z()[e++]]);return Wb(a.createRenderBundleEncoder({colorFormats:c,
depthStencilFormat:W[z()[b+2]],sampleCount:z()[b+3]}),a)},wgpu_device_create_render_pipeline_async:function(a,b,c,d){let e=X[a],h=f=>{M(c)(a,Wb(f,e),d)};e.createRenderPipelineAsync(fc(b)).then(Mb(h)).catch(()=>{h()})},wgpu_device_create_shader_module:function(a,b){var c=X[a],d=c.createShaderModule;var e=b>>2;b=O(z()[e]);var h=e+1;e=y()[h];let f=[];h=z()[h+1]>>2;let k;for(;e--;)k=z()[h+1],f.push({entryPoint:O(z()[h]),layout:1<k?X[k]:k?"auto":null}),h+=2;return Wb(d.call(c,{code:b,compilationHints:f}),
X[a])},wgpu_device_get_queue:function(a){return X[a].queue.B},wgpu_device_set_lost_callback:function(a,b,c){X[a].lost.then(d=>{var e="destroyed"==d.reason?1:0;if(d=d.message){var h=ic(),f=gc(d)+1,k=hc(f);U(d,k,f)}M(b)(a,e,k,c);h&&jc(h)})},wgpu_encoder_end:function(a){X[a].end();Tb(a)},wgpu_encoder_finish:function(a){let b=X[a].finish();Tb(a);return Y(b)},wgpu_encoder_set_bind_group:function(a,b,c,d,e){X[a].setBindGroup(b,X[c],z(),d>>2,e)},wgpu_encoder_set_pipeline:function(a,b){X[a].setPipeline(X[b])},
wgpu_pipeline_get_bind_group_layout:function(a,b){return Y(X[a].getBindGroupLayout(b))},wgpu_queue_submit_one_and_destroy:function(a,b){X[a].submit([X[b]]);Tb(b)},wgpu_queue_write_buffer:function(a,b,c,d,e){X[a].writeBuffer(X[b],c,g(),d,e)},wgpu_render_commands_mixin_draw:function(a,b,c,d,e){X[a].draw(b,c,d,e)},wgpu_render_pass_encoder_execute_bundles:function(a,b,c){X[a].executeBundles(Sb(b,c))},wgpu_texture_create_view_simple:function(a){return Wb(X[a].createView(),X[a])}},Z=function(){function a(c,
d){Z=c.exports;db=Z.__indirect_function_table;Ha.unshift(Z.__wasm_call_ctors);Ca=d;Va();return Z}var b={env:lc,wasi_snapshot_preview1:lc};Ua();if(B.instantiateWasm)try{return B.instantiateWasm(b,a)}catch(c){return I(`Module.instantiateWasm callback failed with error: ${c}`),!1}ab(b,function(c){a(c.instance,c.module)});return{}}();B._renderAdapterInit=a=>(B._renderAdapterInit=Z.renderAdapterInit)(a);
var P=a=>(P=Z.malloc)(a),mc=B._main=(a,b)=>(mc=B._main=Z.main)(a,b),Eb=a=>(Eb=Z.free)(a),Ma=B._emscripten_wasm_worker_initialize=(a,b)=>(Ma=B._emscripten_wasm_worker_initialize=Z.emscripten_wasm_worker_initialize)(a,b),ic=()=>(ic=Z.stackSave)(),jc=a=>(jc=Z.stackRestore)(a),hc=a=>(hc=Z.stackAlloc)(a);B.dynCall_jiji=(a,b,c,d,e)=>(B.dynCall_jiji=Z.dynCall_jiji)(a,b,c,d,e);B.___start_em_js=5468;B.___stop_em_js=6006;
B.dynCall=(a,b,c)=>{a.includes("j")?(a=B["dynCall_"+a],b=c&&c.length?a.apply(null,[b].concat(c)):a.call(null,b)):b=M(b).apply(null,c);return b};B.ccall=kc;B.cwrap=(a,b,c,d)=>{var e=!c||c.every(h=>"number"===h||"boolean"===h);return"string"!==b&&e&&!d?B["_"+a]:function(){return kc(a,b,c,arguments,d)}};var nc;Ta=function oc(){nc||pc();nc||(Ta=oc)};
function pc(){function a(){if(!nc&&(nc=!0,B.calledRun=!0,!Da)){La();Qa(Ia);if(B.onRuntimeInitialized)B.onRuntimeInitialized();if(qc){var b=mc;try{var c=b(0,0);Ea=c;Gb(c)}catch(d){Fb(d)}}if(B.postRun)for("function"==typeof B.postRun&&(B.postRun=[B.postRun]);B.postRun.length;)b=B.postRun.shift(),Ka.unshift(b);Qa(Ka)}}if(!(0<L))if(ra)La();else{if(B.preRun)for("function"==typeof B.preRun&&(B.preRun=[B.preRun]);B.preRun.length;)Ra();Qa(Ga);0<L||(B.setStatus?(B.setStatus("Running..."),setTimeout(function(){setTimeout(function(){B.setStatus("")},
1);a()},1)):a())}}if(B.preInit)for("function"==typeof B.preInit&&(B.preInit=[B.preInit]);0<B.preInit.length;)B.preInit.pop()();var qc=!0;B.noInitialRun&&(qc=!1);pc();
