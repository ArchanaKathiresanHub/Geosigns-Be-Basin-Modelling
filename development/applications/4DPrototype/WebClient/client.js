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

function createCheckBoxDiv(name, checked, changedHandler)
{
    var cb = document.createElement("input");
    cb.type="checkbox";
    cb.name=name;
    cb.checked=checked;
    cb.style.marginLeft="10px";
    cb.onchange=function(){ changedHandler(cb); };

    var label = document.createElement("label");
    var text = document.createTextNode(name);
    label.appendChild(text);

    var div = document.createElement("div");
    div.appendChild(cb);
    div.appendChild(label);

    return div;
}

function initFormations(names)
{
    var formationsDiv = document.getElementById("formationsList");

    for(var i=0; i < names.length; ++i)
        formationsDiv.appendChild(createCheckBoxDiv(names[i], true, onFormationCheckBoxChanged));
}

function initSurfaces(names)
{
    var surfacesDiv = document.getElementById("surfacesList");

    for(var i=0; i < names.length; ++i)
        surfacesDiv.appendChild(createCheckBoxDiv(names[i], false, onSurfaceCheckBoxChanged));
}

function initReservoirs(names)
{
    var reservoirsDiv = document.getElementById("reservoirsList");

    for(var i=0; i < names.length; ++i)
        reservoirsDiv.appendChild(createCheckBoxDiv(names[i], false, onReservoirCheckBoxChanged));
}

function initFaults(collections)
{
    var collectionsDiv = document.getElementById("faultsList");

    for(var i=0; i < collections.length; ++i)
    {
        var collection = collections[i];

        collDiv = document.createElement("div");
        collDiv.name=collection.name;
        collDiv.style.marginLeft="10px";
        collDiv.appendChild(document.createTextNode(collection.name));

        for(var j=0; j < collection.faults.length; ++j)
            collDiv.appendChild(createCheckBoxDiv(collection.faults[j], false, onFaultCheckBoxChanged));

        collectionsDiv.appendChild(collDiv);
    }
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
    initReservoirs(projectInfo.reservoirs);
    initFaults(projectInfo.faultCollections);
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

    var msg = {
            cmd: "EnableFormation", 
            params: {
                name: elem.name, 
                enabled: elem.checked 
            }
        };

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onSurfaceCheckBoxChanged(elem)
{
    console.log("surface " + elem.name + " enabled = " + elem.checked);

    var msg = {
            cmd: "EnableSurface", 
            params: {
                name: elem.name, 
                enabled: elem.checked 
            }
        };

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onReservoirCheckBoxChanged(elem)
{
    console.log("reservoir " + elem.name + " enabled = " + elem.checked);

    var msg = {
        cmd: "EnableReservoir",
        params: {
            name: elem.name,
            enabled: elem.checked
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onFaultCheckBoxChanged(elem)
{
    console.log("fault " + elem.name + " enabled = " + elem.checked);

    var msg = {
        cmd: "EnableFault",
        params: {
            collection: elem.parentNode.parentNode.name,
            name: elem.name,
            enabled: elem.checked
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onPropertyRadioButtonClicked(elem)
{
    console.log("property " + elem.value + " clicked");

    var msg = {
        cmd: "SetProperty",
        params: {
            name: elem.value
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onFlowVizRadioClicked(elem)
{
    console.log("radiobutton " + elem.value + " clicked");

    var msg = {
        cmd: "ShowFlowDirection",
        params: {
            type: elem.value
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onSlicePositionChanged(index, elem)
{
    console.log("sliceI position = " + elem.value);

    var msg = {
        cmd: "SetSlicePosition",
        params: {
            slice: index,
            position: elem.valueAsNumber
        }
    };

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onSliceCheckBoxChanged(index, elem)
{
    console.log("sliceI enabled = " + elem.checked);

    var msg = {
        cmd: "EnableSlice",
        params: {
            slice: index,
            enabled: elem.checked
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onVerticalScaleSliderChanged(elem)
{
    console.log("vertical scale = " + elem.value);

    var msg = {
        cmd: "SetVerticalScale",
        params: {
            scale: elem.valueAsNumber
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onRenderStyleChanged()
{
    var drawFaces = document.getElementById("checkBoxDrawFaces").checked;
    var drawEdges = document.getElementById("checkBoxDrawEdges").checked;

    var msg = {
        cmd: "SetRenderStyle",
        params: {
            drawFaces: drawFaces,
            drawEdges: drawEdges
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onShowGridChanged(elem)
{
    var showGrid = elem.checked;

    var msg = {
        cmd: "ShowCoordinateGrid",
        params: {
            show: showGrid
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onShowTrapsChanged(elem)
{
    var showTraps = elem.checked;

    var msg = {
        cmd: "ShowTraps",
        params: {
            show: showTraps
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onPerspectiveChanged(elem)
{
    var projection = elem.checked 
        ? "Perspective" 
        : "Orthographic";

    var msg = {
        cmd: "SetProjection",
        params: {
            type: projection
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onTimeSliderChanged(elem)
{
    console.log("timeSlider = " + elem.value);

    var msg = {
        cmd: "SetCurrentSnapshot",
        params: { 
            index: elem.valueAsNumber
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onButtonViewAllClicked()
{
    console.log("view all");

    var msg = {
        cmd: "ViewAll",
        params: {}
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onButtonViewPresetClicked(index)
{
    var viewDirs = [ "Top", "Left", "Front", "Bottom", "Right", "Back" ];

    console.log("view preset " + viewDirs[index]);

    var msg = {
        cmd: "SetViewPreset",
        params: {
            preset: viewDirs[index]
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onQualitySliderChanged(value)
{
    var msg = {
        cmd: "SetStillQuality",
        params: {
            quality: value
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onInteractiveQualitySliderChanged(value)
{
    var msg = {
        cmd: "SetInteractiveQuality",
        params: {
            quality: value
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
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

function measurebandwithandfps()
{
    // refresh the bandwidth and the fps
    if (node != null)
    {
        node.innerHTML = '<p>Bandwidth : ' + dataSize / 1000 + ' kb/s</p><p>FPS : ' + fps + '</p>';
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
    url += ":8080/";

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

    theRenderArea.addReceivedImageListener(receivedImage);
    theRenderArea.addMessageListener(receivedMessage);

    // Connects to the service. The IP address and the port refer to those of the service 
    // (see main.cpp). "Model" refers to the name of the requested renderArea.
    var url = websocketURL() + generateGUID();
    theRenderArea.connectTo(url);

    // Calls a function or executes a code snippet repeatedly to refresh the bandwidth and the fps
    window.setInterval("measurebandwithandfps()",1000);
	
    node = document.getElementById('bandwidthfps');
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
    {
        var msg = {
            cmd: "SetWidth",
            params: {
                width: $("#rwidth").val()
            }
        }

	   theRenderArea.sendMessage(JSON.stringify(msg));
    }
}

function rheight(){
    // call when changing the renderArea height
    if ($("#rheight").val() != "")
    {
        var msg = {
            cmd: "SetHeight",
            params: {
                height: $("#rheight").val()
            }
        }
	   
        theRenderArea.sendMessage(JSON.stringify(msg));
    }
}
