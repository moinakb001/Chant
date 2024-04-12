// If absolute URL from the remote server is provided, configure the CORS
// header on that server.
var url = './test.pdf';

// Loaded via <script> tag, create shortcut to access PDF.js exports.
var { pdfjsLib } = globalThis;
var canvas = document.getElementById('the-canvas');

var pixrat = window.devicePixelRatio || 1;

canvas.width = canvas.clientWidth * pixrat;
canvas.height = canvas.clientHeight * pixrat;
// Prepare canvas using PDF page dimensions
var context = canvas.getContext('2d');

// The workerSrc property shall be specified.
pdfjsLib.GlobalWorkerOptions.workerSrc = './lib/pdfjs/pdf.worker.mjs';

// Asynchronous download of PDF
var pdf = await pdfjsLib.getDocument(url).promise;

// Fetch the first page
var pageNumber = pdf.numPages;
var pagePromises = [];
for (var i = 0; i < pageNumber; i++) {
    pagePromises[i] = pdf.getPage(i + 1);
}
var pages = await Promise.all(pagePromises);
var viewports = pages.map((page) => page.getViewport({ scale: pixrat }));
var prevRender = undefined;


var nextVp = {
    x: 100,
    y: 200,
    page: 0,
    scale: 1
};
var curRender = 0;
canvas.addEventListener('wheel' ,(evt)=>
{
    nextVp.y += evt.deltaY;
    if (nextVp.y < 0)
    {
        if(nextVp.page == 0)
        {
            nextVp.y = 0;
        }
        else
        {
            nextVp.page -= 1;
            nextVp.y += viewports[nextVp.page].height;
        }
    }
    if (nextVp.y > viewports[nextVp.page].height)
    {
        if(nextVp.page == pages.length - 1)
        {
            nextVp.y = viewports[nextVp.page].height;
        }
        else
        {
            nextVp.y -= viewports[nextVp.page].height;
            nextVp.page += 1;
        }
    }

    queueRender();
    evt.preventDefault();
});

var renderFn = async () => {
    var curVp = nextVp;
    var curH = 0;
    var curPage = curVp.page
    while(curH<canvas.clientHeight && curPage != pages.length)
    {
        var transform = [curVp.scale, 0, 0, curVp.scale, -curVp.x, curH-curVp.y];
        curH += viewports[curPage].height;
        var renderContext = {
            canvasContext: context,
            transform: transform,
            viewport: viewports[curPage]
        };
        await pages[curPage].render(renderContext).promise;
        curPage++;
    }
    curRender -= 1;
};

var queueRender = () => {
    if (curRender == 0)
    {
        curRender += 1;
        prevRender = renderFn();
    }
    else if(curRender == 1)
    {
        curRender += 1;
        prevRender = prevRender.then(renderFn());
    }
}

window.addEventListener("resize", (event) => {
    console.log('resize called', pixrat);
    canvas.width = canvas.clientWidth * pixrat;
    canvas.height = canvas.clientHeight * pixrat;
    queueRender();
});

var animCb = (num) =>
{
    nextVp.x += num/1000;

    queueRender();
    window.requestAnimationFrame(animCb);
}
//window.requestAnimationFrame(animCb);



queueRender();


