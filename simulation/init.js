/*
 * This code searches for all the <script type="application/processing" target="canvasid">
 * in your page and loads each script in the target canvas with the proper id.
 * It is useful to smooth the process of adding Processing code in your page and starting
 * the Processing.js engine.
 */

if ( window.addEventListener ) {
	window.addEventListener("load", function() {
		var canvases = document.getElementsByTagName("canvas");
		for ( var i = 0; i < canvases.length; i++ ) {
			source_element = document.getElementById(canvases[i].getAttribute('src'));
			source = source_element.firstChild.nodeValue;	
			Processing(canvases[i], source);
		}
	}, false);
}
