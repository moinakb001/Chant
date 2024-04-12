"use strict"


import * as mupdf from "./lib/mupdf/mupdf.js"

var pdf = undefined;
self.onmessage = (e) => {
    switch(e.data.type)
    {
        case 0:
            console.log("loading pdf...");
            pdf = mupdf.Document.openDocument(e.data.data, "application/pdf");
            postMessage(2);
            break;
        case 1:
            var info = e.data.data;
            console.log("loading page ", info.page);
            var data = pdf.loadPage(info.page).toPixmap([info.scale * info.devpix * 96/72,0,0,info.scale * info.devpix*96/72,0,0], mupdf.ColorSpace.DeviceRGB).asPNG();
            var blob = new Blob([data], { type: "image/png" });
            createImageBitmap(blob).then((imgd) => {
                postMessage({
                    idx: info.page,
                    scale: info.scale,
                    data: imgd
                }, [imgd]);
            });
            break;
    }
}
postMessage(1);
