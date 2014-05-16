var theRenderArea = null;
var theCanvasDiv = null;
var theCanvas = null;
var node = null;
var color = false;
var dataSize = 0;
var fps = 0;		
var resizeTimer = null;
var leftMargin = 300;
var bottomMargin = 100;

function resizeCanvas()
{
	var w = window.innerWidth - leftMargin;
	var h = window.innerHeight - bottomMargin;
	theRenderArea.requestRenderAreaSize(w, h);
	theRenderArea.resizeRenderAreaContainer(w, h);
}

function onWindowResize()
{
	if(resizeTimer)
		clearTimeout(resizeTimer);
	resizeTimer = setTimeout(resizeCanvas, 1000);
}

function receivedImage(length){
	// call when receiving a new image from the service
	dataSize += length;
	fps += 1;
}

function receivedMessage(message)
{
	var msgObj = JSON.parse(message);
	if(msgObj.projectInfo)
	{
		theRenderArea.projectInfo = msgObj.projectInfo;

		$("#slider_snapshot").prop('max', msgObj.projectInfo.snapshotCount - 1);
		$("#slider_sliceI").prop('max', msgObj.projectInfo.numI - 1);
		$("#slider_sliceJ").prop('max', msgObj.projectInfo.numJ - 1);

		for(var i=0; i < msgObj.projectInfo.properties.length; ++i)
		{
			$("#properties").append("<option>" + msgObj.projectInfo.properties[i] + "</option>");
		}
	}
}

function measurebandwithandfps(){
	// refresh the bandwidth and the fps
	if (node != null)
	{
		node.innerHTML = '<h3>Bandwidth : ' + dataSize / 1000 + ' kb/s</h3><h3>FPS : ' + fps + '</h3>';
		dataSize = 0;
		fps = 0;
	}
}

function generateGUID()
{
	var guid = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) 
	{
    	var r = Math.random()*16|0, v = c == 'x' ? r : (r&0x3|0x8);
    	return v.toString(16);
	});

	return guid;
}

/**
 * Construct the url for the websocket to connect to. Use localhost if the current
 * page was loaded from file, otherwise connect to the same host that served the page
 */
function websocketURL()
{
	var loc = window.location, url;

	if(loc.protocol === "file:")
		url = "ws://localhost";
	else
		url = "ws://" + loc.hostname;

	// add port
	url += ":8081/";

	return url;
}

function init() 
{ 
	window.canvas = $("#TheCanvas");
	$(window).resize(onWindowResize);

	//var defaultWidth  = 2560;
	//var defaultHeight = 1440;
	// This function is called immediately after the page is loaded. Initialization of 
	// the renderArea. "TheCanvas" refers to the id of the canvas. 
    theRenderArea = new RemoteVizRenderArea("TheCanvas", //defaultWidth, defaultHeight);
    	window.innerWidth - leftMargin, 
    	window.innerHeight - bottomMargin);

	// add a listener on the receivedImage event.
	theRenderArea.addReceivedImageListener(receivedImage);
	// add a listener for messages from the server
	theRenderArea.addMessageListener(receivedMessage);

	// Connects to the service. The IP address and the port refer to those of the service 
	// (see main.cpp). "Model" refers to the name of the requested renderArea.
	var url = websocketURL() + generateGUID();
    theRenderArea.connectTo(url);

	// Bind an event handler to the "slidestop" JavaScript event, or trigger that event on an element.
	$("#slider_maxfps").on('slidestop', maxfps);
	$("#slider_squality").on('slidestop', squality);
	$("#slider_iquality").on('slidestop', iquality);
	$("#slider_scalefactor").on('slidestop', scalefactor);
	$("#slider_snapshot").on('slidestop', snapshot);
	$("#slider_vscale").on('slidestop', vscale);
	$("#slider_sliceI").on('slidestop', sliceI);
	$("#slider_sliceJ").on('slidestop', sliceJ);
	$("[name='rendermode']").change(renderModeChanged);
	$("[name='meshmode']").change(meshModeChanged);
	$("#properties").change(propertyChanged);
	$('#checkbox_faces').change(drawFacesChanged);
	$('#checkbox_edges').change(drawEdgesChanged);
	$("#button_viewall").click(function() { theRenderArea.sendMessage("VIEWALL") });
	// Calls a function or executes a code snippet repeatedly to refresh the bandwidth and the fps
	window.setInterval("measurebandwithandfps()",1000);
	
	node = document.getElementById('bandwidthfps');
}

function sliceI()
{
	index = $("#slider_sliceI").val();
	theRenderArea.sendMessage("SLICEI " + index);
}

function sliceJ()
{
	index = $("#slider_sliceJ").val();
	theRenderArea.sendMessage("SLICEJ " + index);
}

function vscale()
{
	scale = $("#slider_vscale").val();
	theRenderArea.sendMessage("VSCALE " + scale);
}

function meshModeChanged()
{
	meshMode = $("[name='meshmode']:checked").val();

	if(meshMode == "formations")
	{
		$("#slider_sliceI").prop('max', theRenderArea.projectInfo.numI - 1);
		$("#slider_sliceJ").prop('max', theRenderArea.projectInfo.numJ - 1);
	}
	else
	{
		$("#slider_sliceI").prop('max', theRenderArea.projectInfo.numIHiRes - 1);
		$("#slider_sliceJ").prop('max', theRenderArea.projectInfo.numJHiRes - 1);
	}

	theRenderArea.sendMessage("MESHMODE " + meshMode);
}

function renderModeChanged()
{
	renderMode = $("[name='rendermode']:checked").val();
	theRenderArea.sendMessage("RENDERMODE " + renderMode);
}

function propertyChanged()
{
	var prop = $("#properties").prop("value");
	theRenderArea.sendMessage("SETPROPERTY " + prop);
}

function drawFacesChanged() 
{
	var arg = $(this).prop('checked') ? "TRUE":"FALSE";
	theRenderArea.sendMessage("DRAWFACES " + arg);
}

function drawEdgesChanged()
{
	var arg = $(this).prop('checked') ? "TRUE":"FALSE";
	theRenderArea.sendMessage("DRAWEDGES " + arg);
}

function cwidth(){
	// call when changing the canvas width
	if ($("#cwidth").val() != ""){
		document.getElementById("TheCanvas").style.width = $("#cwidth").val();
		theRenderArea.resizeRenderAreaContainer($("#cwidth").val(), $("#cheight").val());
	}
}

function cheight(){
	// call when changing the canvas height
	if ($("#cheight").val() != ""){
		document.getElementById("TheCanvas").style.height = $("#cheight").val();
		theRenderArea.resizeRenderAreaContainer($("#cwidth").val(), $("#cheight").val());
	}
}

function rwidth(){
	// call when changing the renderArea width
	if ($("#rwidth").val() != "")
		theRenderArea.sendMessage("WIDTH " + $("#rwidth").val());
}

function rheight(){
	// call when changing the renderArea height
	if ($("#rheight").val() != "")
		theRenderArea.sendMessage("HEIGHT " + $("#rheight").val());
}

function maxfps(){
	// call when moving the max fps slider
	theRenderArea.sendMessage("FPS " + $("#slider_maxfps").val());
}

function squality(){
	// call when moving the still quality slider
	theRenderArea.sendMessage("STILLQUALITY " + $("#slider_squality").val());
}

function iquality(){
	// call when moving the interactive quality slider
	theRenderArea.sendMessage("INTERACTIVEQUALITY " + $("#slider_iquality").val());
}

function scalefactor(){
	// call when moving the scale factor slider
	theRenderArea.sendMessage("SCALEFACTOR " + $("#slider_scalefactor").val());
}
	
function snapshot(){
	theRenderArea.sendMessage("SNAPSHOT " + $("#slider_snapshot").val());
}
