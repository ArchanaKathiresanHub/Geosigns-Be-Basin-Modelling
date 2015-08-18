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
var timestamp = 0;

function showTab(index)
{
    var elems = [
        document.getElementById('formations'),
        document.getElementById('properties')];

    var tabs = [
        document.getElementById('structureTab'),
        document.getElementById('propertiesTab')];

    elems[index].style.display = "block";
    elems[1 - index].style.display = "none";

    tabs[index].className="activeTab";
    tabs[1-index].className="tab";
}

function initFormations(names)
{
    var fmt = "<div><input type='checkbox' name='xxx' checked onchange='onFormationCheckBoxChanged(this)'>xxx</input></div>";
    var formationsDiv = document.getElementById("formationsList");

    for(var i=0; i < names.length; ++i)
        formationsDiv.innerHTML += fmt.replace(/xxx/g, names[i]);
}

function initSurfaces(names)
{
    var fmt = "<div><input type='checkbox' name='xxx' onchange='onSurfaceCheckBoxChanged(this)'>xxx</input></div>";
    var surfacesDiv = document.getElementById("surfacesList");

    for(var i=0; i < names.length; ++i)
        surfacesDiv.innerHTML += fmt.replace(/xxx/g, names[i]);
}

function initProperties(names)
{
    var fmt = "<div><input type='radio' name='property' value='xxx' onclick='onPropertyRadioButtonClicked(this)'>xxx</input></div>";
    var propertiesList = document.getElementById("propertiesList");

    for(var i=0; i < names.length; ++i)
        propertiesList.innerHTML += fmt.replace(/xxx/g, names[i]);
}

function initUI(projectInfo)
{
    initFormations(projectInfo.formations);
    initSurfaces(projectInfo.surfaces);
    initProperties(projectInfo.properties);

    var sliceI = document.getElementById("sliceISlider");
    sliceI.min = 0;
    sliceI.max = projectInfo.numI - 2;
    sliceI.step = 1;

    var sliceJ = document.getElementById("sliceJSlider");
    sliceJ.min = 0;
    sliceJ.max = projectInfo.numJ - 2;
    sliceJ.step = 1;

    var timeSlider = document.getElementById("timeSlider");
    timeSlider.min = 0;
    timeSlider.max = projectInfo.snapshotCount - 1;
    timeSlider.step = 1;
}

function onFormationCheckBoxChanged(elem)
{
    console.log("formation " + elem.name + " enabled = " + elem.checked);

    var enabled = (elem.checked ? "TRUE" : "FALSE");
    theRenderArea.sendMessage("ENABLEFORMATION " + elem.name + " " + enabled);
}

function onSurfaceCheckBoxChanged(elem)
{
    console.log("surface " + elem.name + " enabled = " + elem.checked);
    var enabled = (elem.checked ? "TRUE" : "FALSE");
    theRenderArea.sendMessage("ENABLESURFACE " + elem.name + " " + enabled);
}

function onPropertyRadioButtonClicked(elem)
{
    console.log("property " + elem.value + " clicked");
    theRenderArea.sendMessage("SETPROPERTY " + elem.value);
}

function onSliceIPositionChanged(elem)
{
    console.log("sliceI position = " + elem.value);
    theRenderArea.sendMessage("SLICEI " + elem.value)
}

function onSliceICheckBoxChanged(elem)
{
    console.log("sliceI enabled = " + elem.checked);
    var arg = elem.checked ? "TRUE" : "FALSE";
    theRenderArea.sendMessage("ENABLESLICEI " + arg);
}

function onSliceJPositionChanged(elem)
{
    console.log("sliceJ position = " + elem.value);
    theRenderArea.sendMessage("SLICEJ " + elem.value)
}

function onSliceJCheckBoxChanged(elem)
{
    console.log("sliceJ enabled = " + elem.checked);
    var arg = elem.checked ? "TRUE" : "FALSE";
    theRenderArea.sendMessage("ENABLESLICEJ " + arg);
}

function onVerticalScaleSliderChanged(elem)
{
    console.log("vertical scale = " + elem.value);
    theRenderArea.sendMessage("VSCALE " + elem.value);
}

function onDrawFacesCheckBoxChanged(elem)
{
    console.log("draw faces = " + elem.checked);
    var arg = elem.checked ? "TRUE":"FALSE";
    theRenderArea.sendMessage("DRAWFACES " + arg);
}

function onDrawEdgesCheckBoxChanged(elem)
{
    console.log("draw edges = " + elem.checked);
    var arg = elem.checked ? "TRUE":"FALSE";
    theRenderArea.sendMessage("DRAWEDGES " + arg);
}

function onTimeSliderChanged(elem)
{
    console.log("timeSlider = " + elem.value);
    theRenderArea.sendMessage("SNAPSHOT " + elem.value);
}

function onButtonViewAllClicked()
{
    console.log("view all");
    theRenderArea.sendMessage("VIEWALL");
}

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
    if(timestamp != 0)
    {
    	var newtimestamp = window.performance.now();
    	console.log("latency: " + (newtimestamp - timestamp));
    	timestamp = 0;
    }
}

function receivedMessage(message)
{
    var msgObj = JSON.parse(message);
    if(msgObj.projectInfo)
    {
    	theRenderArea.projectInfo = msgObj.projectInfo;
        initUI(msgObj.projectInfo);
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
    window.canvas = document.getElementById("TheCanvas");
    //$(window).resize(onWindowResize);

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

function propertyChanged()
{
    var prop = $("#properties").prop("value");
    theRenderArea.sendMessage("SETPROPERTY " + prop);
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

function snapshot(){
    timestamp = window.performance.now();
    theRenderArea.sendMessage("SNAPSHOT " + $("#slider_snapshot").val());
}
