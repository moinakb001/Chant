"use strict"

//import * as mupdf from "./lib/mupdf/mupdf.js"
// If absolute URL from the remote server is provided, configure the CORS
// header on that server.
var url = './test.pdf';
import * as mupdf from "./lib/mupdf/mupdf.js"

var devPix = window.devicePixelRatio || 1;

var canvas = document.getElementById("the-canvas");
var ctx = canvas.getContext("2d");
canvas.width = canvas.clientWidth * devPix;
canvas.height = canvas.clientHeight * devPix;


var maxWorkers = navigator.hardwareConcurrency;
var workers = [];
var outstandingJobs = [];
var imgs = [];
var workersInited = [];
var curView = {
    page: 0,
    x: 0,
    y: 0,
    scale: 1
};

var fileData = await (await fetch(url)).arrayBuffer();
var pdf = mupdf.Document.openDocument(fileData, "application/pdf");
var numP = pdf.countPages();
var pageMeta = [];
var scaledHeight = (i) => pageMeta[i].height / 72*96 * devPix * curView.scale;
var scaledWidth = (i) => pageMeta[i].width / 72*96 * devPix * curView.scale;
var acceptableScale = (a, b) => (a > (0.9 * b)) && (a < (1.5 * b));
for(var i = 0; i < numP; i++)
{
    var arr = pdf.loadPage(i).getBounds();
    pageMeta[i] = {
        height: arr[3] - arr[1],
        width: arr[2] - arr[0]
    };
    console.log(pageMeta[i]);
}
var needAnim = 0;
var renderFn = (delta) => {
    needAnim = 0;
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    var curP = curView.page;
    var curH = 0;
    var cu = imgs[curP];
    while(curH-curView.y < canvas.height)
    {
        if(cu.scale != 0)
        {
            ctx.drawImage(cu.data, -curView.x, curH-curView.y, scaledWidth(curP), scaledHeight(curP) );
        }
        if(!acceptableScale(cu.scale, curView.scale) && cu.disp != 1)
        {
            cu.disp = 1;
            dispatchWork(curP, curView.scale);
        }
        curH += scaledHeight(curP);
        curP++;
        cu = imgs[curP];
    }
};
var queueAnim = () => {
    if(needAnim != 0) return;
    needAnim = 1;
    window.requestAnimationFrame(renderFn);
}
for(var i = 0; i < maxWorkers; i++)
{
    ((cur) => {
        workers[cur] = new Worker("./worker.mjs", {type: "module"});
        workersInited[cur] = new Promise( (resolve, reject) =>{
            workers[cur].onmessage =  (e) => {
                if(e.data == 1)
                {
                    workers[cur].postMessage({
                        type: 0,
                        data: fileData
                    });
                    return;
                }
                if(e.data == 2)
                {
                    resolve();
                    return;
                }
                outstandingJobs[cur]--;
                imgs[e.data.idx] = {
                    scale: e.data.scale,
                    data: e.data.data
                };
                queueAnim();
            };
        });
        outstandingJobs[cur] = 0;
    })(i);
}
await Promise.all(workersInited);

var dispatchWork = (page, scale) =>
{
    var minJobs = numP * 100;
    var cur = -1;
    for(var i = 0; i < maxWorkers; i++)
    {
        if(outstandingJobs[i] < minJobs)
        {
            minJobs = outstandingJobs[i];
            cur = i;
        }
    }
    outstandingJobs[cur]++;
    workers[cur].postMessage({
        type: 1,
        data: {
            page: page,
            scale: scale,
            devpix: devPix
        }
    });
};
for(var i = 0; i < numP; i++)
{
    imgs[i] = {scale: 0, data: 0};
}
canvas.addEventListener("wheel", (event) => {
    event.preventDefault();
    curView.y += event.deltaY * curView.scale;
    curView.x += event.deltaX * curView.scale;
    if(curView.y >= scaledHeight(curView.page))
    {
        if(curView.page == pageMeta.length - 1)
        {
            curView.y = scaledHeight(curView.page);
        }
        else
        {
            curView.y -= scaledHeight(curView.page);
            curView.page++;
        }
    }
    else if(curView.y < 0)
    {
        if(curView.page == 0)
        {
            curView.y = 0;
        }
        else
        {
            curView.y += scaledHeight(curView.page);
            curView.page--;
        }
    }
    queueAnim();
});
queueAnim();
window.onresize = (e) => {
    canvas.height = canvas.clientHeight * window.devicePixelRatio;
    canvas.width = canvas.clientWidth * window.devicePixelRatio;
    curView.scale = window.devicePixelRatio / devPix;
    queueAnim();
    console.log(e);
}

var curTouches = [0, 0];
var curTouchNum = 0;
function copyTouch({ identifier, pageX, pageY }) {
    return { identifier, pageX, pageY };
  }

canvas.addEventListener("touchstart", (e) => {
    e.preventDefault();
    var i = 0;
    while(curTouchNum < 2 && i < e.changedTouches.length)
    {
        if(curTouchNum==1 && e.changedTouches[i].identifier == curTouches[0].identifier) {i++; continue;}
        curTouches[curTouchNum] = copyTouch(e.changedTouches[i]);
        i++;
        curTouchNum++;
    }
});
var centroid = (a, b) => {return {pageX: (a.pageX + b.pageX) / 2, pageY: (a.pageY + b.pageY) / 2}};
canvas.addEventListener("touchmove", (e) => {
    e.preventDefault();
    var init = curTouches[0];
    ctx.fillStyle = "blue";
ctx.fillRect(0, 0, canvas.width, canvas.height);
    if(curTouchNum == 2)
    {
        init = centroid(curTouches[0], curTouches[1]);
    }
    ctx.fillStyle = "red";
ctx.fillRect(0, 0, canvas.width, canvas.height);
    var arr = [0, 0];
    for(var t of e.changedTouches)
    {
        if(t.identifier == curTouches[0].identifier)
        {
            arr[0] = copyTouch(t);
        }
        else if(t.identifier == curTouches[1].identifier)
        {
            arr[1] = copyTouch(t);
        }
    }
    var fini = arr[0];
    ctx.fillStyle = "green";
ctx.fillRect(0, 0, canvas.width, canvas.height);
    if(curTouchNum == 2)
    {
        fini = centroid(arr[0], arr[1]);
    }
    ctx.fillStyle = "yellow";
ctx.fillRect(0, 0, canvas.width, canvas.height);
    curTouches = arr;
    
    curView.y += (init.pageY - fini.pageY) * curView.scale * devPix;
    curView.x += (init.pageX - fini.pageX) * curView.scale * devPix;
    if(curView.y >= scaledHeight(curView.page))
    {
        if(curView.page == pageMeta.length - 1)
        {
            curView.y = scaledHeight(curView.page);
        }
        else
        {
            curView.y -= scaledHeight(curView.page);
            curView.page++;
        }
    }
    else if(curView.y < 0)
    {
        if(curView.page == 0)
        {
            curView.y = 0;
        }
        else
        {
            curView.y += scaledHeight(curView.page);
            curView.page--;
        }
    }
    queueAnim();

});
canvas.addEventListener("touchend", (e) => {
    e.preventDefault();
    for(var t of e.changedTouches)
    {
        for(var i = 0; i < curTouchNum; i++)
        {
            if(t.identifier == curTouches[i].identifier)
            {
                curTouches[0] = curTouches[i];
                curTouchNum--;
                break;
            }
        }
    }
});