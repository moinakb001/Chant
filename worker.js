"use strict"

import * as mupdf from "./lib/mupdf/mupdf.js"
// If absolute URL from the remote server is provided, configure the CORS
// header on that server.
var url = './test.pdf';

// Loaded via <script> tag, create shortcut to access PDF.js exports.
/*var canvas = document.getElementById('the-canvas');

var pixrat = window.devicePixelRatio || 1;

canvas.width = canvas.clientWidth * pixrat;
canvas.height = canvas.clientHeight * pixrat;
// Prepare canvas using PDF page dimensions
var context = canvas.getContext('2d');*/
console.log(mupdf);