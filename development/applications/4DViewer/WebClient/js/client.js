var theRenderArea = null;
var bandwidthDiv = null;
var fpsDiv = null;
var dataSize = 0;
var fps = 0;		
var resizeTimer = null;
var leftMargin = 330;
var bottomMargin = 20;
var timestamp = 0;

var logMessages = true;

function sendMsg(msg)
{
    var msgStr = JSON.stringify(msg, null, 4);
    
    if(logMessages)
	console.log(msgStr);
    
    theRenderArea.sendMessage(msgStr);
}

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

function createCheckBoxDiv(name, objectId, checked, changedHandler)
{
    var cb = document.createElement("input");
    cb.type="checkbox";
    cb.name=name;
    cb.checked=checked;
    cb.style.marginLeft="20px";
    cb.onchange=function(){ changedHandler(cb, objectId); };

    var label = document.createElement("label");
    var text = document.createTextNode(name);
    label.appendChild(text);

    var div = document.createElement("div");
    div.appendChild(cb);
    div.appendChild(label);

    return div;
}

function createRadioButtonDiv(groupName, name, objectId, changedHandler)
{
    var rb = document.createElement("input");
    rb.type="radio";
    rb.name=groupName;
    rb.style.marginLeft="20px";
    rb.onchange=function(){ changedHandler(rb, objectId); };

    var label = document.createElement("label");
    var text = document.createTextNode(name);
    label.appendChild(text);

    var div = document.createElement("div");
    div.appendChild(rb);
    div.appendChild(label);

    return div;
}

function initFormations(names)
{
    var formationsDiv = document.getElementById("formationsList");

    for(var i=0; i < names.length; ++i)
        formationsDiv.appendChild(createCheckBoxDiv(names[i], i, true, onFormationCheckBoxChanged));
}

function initSurfaces(names)
{
    var surfacesDiv = document.getElementById("surfacesList");

    for(var i=0; i < names.length; ++i)
        surfacesDiv.appendChild(createCheckBoxDiv(names[i], i, false, onSurfaceCheckBoxChanged));
}

function initReservoirs(names)
{
    var reservoirsDiv = document.getElementById("reservoirsList");

    for(var i=0; i < names.length; ++i)
        reservoirsDiv.appendChild(createCheckBoxDiv(names[i], i, false, onReservoirCheckBoxChanged));
}

function initFaults(collections)
{
    var collectionsDiv = document.getElementById("faultsList");

    var objectId=0;
    for(var i=0; i < collections.length; ++i)
    {
        var collection = collections[i];

        collDiv = document.createElement("div");
        collDiv.name=collection.name;
        collDiv.style.marginLeft="10px";
        collDiv.appendChild(document.createTextNode(collection.name));

        for(var j=0; j < collection.faults.length; ++j)
            collDiv.appendChild(createCheckBoxDiv(collection.faults[j], objectId++, false, onFaultCheckBoxChanged));

        collectionsDiv.appendChild(collDiv);
    }
}

function initFlowLines(names)
{
    var flowLinesDiv = document.getElementById("flowLinesList");

    for(var i=0; i < names.length; ++i)
        flowLinesDiv.appendChild(createCheckBoxDiv(names[i], i, false, onFlowLinesCheckBoxChanged));
}

function initProperties(names)
{
    //var fmt = "<div><input type='radio' name='property' value='xxx' onclick='onPropertyRadioButtonClicked(this)'>xxx</input></div>";
    var propertiesList = document.getElementById("propertiesList");

    //for(var i=0; i < names.length; ++i)
    //    propertiesList.innerHTML += fmt.replace(/xxx/g, names[i]);

    for(var i=0; i < names.length; ++i)
        propertiesList.appendChild(createRadioButtonDiv("properties", names[i], i, onPropertyRadioButtonClicked));
}

function initUI(projectInfo)
{
    initFormations(projectInfo.formations);
    initSurfaces(projectInfo.surfaces);
    initReservoirs(projectInfo.reservoirs);
    initFaults(projectInfo.faultCollections);
    initFlowLines(projectInfo.flowLines);
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
    timeSlider.value = 0;

    // Init JPEG quality
    onQualitySliderChanged(document.getElementById("qualitySlider").valueAsNumber);
    onInteractiveQualitySliderChanged(document.getElementById("iqualitySlider").valueAsNumber);
    // Init H264 quality
    //onBandwidthSliderChanged(document.getElementById("bandwidthSlider").valueAsNumber * 8192);
    onMaxFPSSliderChanged(document.getElementById("maxfpsSlider").valueAsNumber);
}

function initSeismicUI(seismicInfo)
{
    var inlineSlider = document.getElementById("sliceInlineSlider");
    inlineSlider.min = seismicInfo.extent.min[1];
    inlineSlider.max = seismicInfo.extent.max[1];

    var crosslineSlider = document.getElementById("sliceCrosslineSlider");
    crosslineSlider.min = seismicInfo.extent.min[2];
    crosslineSlider.max = seismicInfo.extent.max[2];
}

function onCheckBoxAllFormationsChanged(elem)
{
    var msg = {
        cmd: "EnableAllFormations",
        params: {
            enabled: elem.checked
        }
    }

    sendMsg(msg);

    var formationsDiv = document.getElementById("formationsList");
    var checkBoxes = formationsDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onCheckBoxAllSurfacesChanged(elem)
{
    var msg = {
        cmd: "EnableAllSurfaces",
        params: {
            enabled: elem.checked
        }
    }

    sendMsg(msg);

    var surfacesDiv = document.getElementById("surfacesList");
    var checkBoxes = surfacesDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onCheckBoxAllReservoirsChanged(elem)
{
    var msg = {
        cmd: "EnableAllReservoirs",
        params: {
            enabled: elem.checked
        }
    }

    sendMsg(msg);

    var reservoirsDiv = document.getElementById("reservoirsList");
    var checkBoxes = reservoirsDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onCheckBoxAllFaultsChanged(elem)
{
    var msg = {
        cmd: "EnableAllFaults",
        params: {
            enabled: elem.checked
        }
    }

    sendMsg(msg);

    var faultsDiv = document.getElementById("faultsList");
    var checkBoxes = faultsDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onCheckBoxAllFlowLinesChanged(elem)
{
    var msg = {
        cmd: "EnableAllFlowLines",
        params: {
            enabled: elem.checked
        }
    }

    sendMsg(msg);

    var flowLinesDiv = document.getElementById("flowLinesList");
    var checkBoxes = flowLinesDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onFormationCheckBoxChanged(elem, objectId)
{
    console.log("formation " + elem.name + " enabled = " + elem.checked);

    var msg = {
            cmd: "EnableFormation", 
            params: {
                formationId: objectId,
                enabled: elem.checked 
            }
        };

    sendMsg(msg);
}

function onSurfaceCheckBoxChanged(elem, objectId)
{
    console.log("surface " + elem.name + " enabled = " + elem.checked);

    var msg = {
            cmd: "EnableSurface", 
            params: {
                surfaceId: objectId,
                enabled: elem.checked 
            }
        };

    sendMsg(msg);
}

function onReservoirCheckBoxChanged(elem, objectId)
{
    console.log("reservoir " + elem.name + " enabled = " + elem.checked);

    var msg = {
        cmd: "EnableReservoir",
        params: {
            reservoirId: objectId,
            enabled: elem.checked
        }
    }

    sendMsg(msg);
}

function onFaultCheckBoxChanged(elem, objectId)
{
    console.log("fault " + elem.name + " enabled = " + elem.checked);

    var msg = {
        cmd: "EnableFault",
        params: {
            faultId: objectId,
            enabled: elem.checked
        }
    }

    sendMsg(msg);
}

function onFlowLinesCheckBoxChanged(elem, objectId)
{
    var msg = {
        cmd: "EnableFlowLines",
        params: {
            flowLinesId: objectId,
            enabled: elem.checked
        }
    }

    sendMsg(msg);
}

function onFenceCheckBoxChanged(elem, objectId)
{
    console.log("fence " + elem.name + " enabled = " + elem.checked);

    var msg = {
            cmd: "EnableFence", 
            params: {
                fenceId: objectId,
                enabled: elem.checked 
            }
        };

    sendMsg(msg);
}

function onPropertyRadioButtonClicked(elem, objectId)
{
    console.log("property " + elem.value + " clicked");

    var msg = {
        cmd: "SetProperty",
        params: {
            propertyId: objectId
        }
    }

    sendMsg(msg);
}

function onShowFlowVectorsChanged(elem)
{
    var msg = {
        cmd: "ShowFlowVectors",
        params: {
            show: elem.checked
        }
    }

    sendMsg(msg);
}

function onFlowLinesStepSliderChanged(elem)
{
    var typeStr = (elem.id == "sliderFlowLinesLeakageStep")
        ? "FlowLinesLeakage"
        : "FlowLinesExpulsion";

    var msg = {
        cmd: "SetFlowLinesStep",
        params: {
            type: typeStr,
            step: elem.valueAsNumber
        }
    }

    sendMsg(msg);
}

function onFlowLinesThresholdSliderChanged(elem)
{
    var typeStr = (elem.id == "sliderFlowLinesLeakageThreshold")
        ? "FlowLinesLeakage"
        : "FlowLinesExpulsion";

    var maxPower = 9.0;
    var thresholdVal = Math.pow(10, maxPower * .01 * elem.valueAsNumber);

    var msg = {
        cmd: "SetFlowLinesThreshold",
        params: {
            type: typeStr,
            threshold: thresholdVal
        }
    }

    theRenderArea.sendMessage(JSON.stringify(msg));
}

function onDrainageAreaRadioClicked(elem)
{
    console.log("radiobutton " + elem.value + " clicked");

    var msg = {
        cmd: "ShowDrainageAreaOutline",
        params: {
            type: elem.value
        }
    }

    sendMsg(msg);
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

    sendMsg(msg);
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

    sendMsg(msg);
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

    sendMsg(msg);
}

function onTransparencySliderChanged(elem)
{
    var transparency = elem.valueAsNumber / elem.max;
    console.log("transparency = " + transparency);

    var msg = {
        cmd: "SetTransparency",
        params: {
            transparency: transparency
        }
    }

    sendMsg(msg);
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

    sendMsg(msg);
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

    sendMsg(msg);
}

function onShowCompassChanged(elem)
{
    var showCompass = elem.checked;

    var msg = {
        cmd: "ShowCompass",
        params: {
            show: showCompass
        }
    }

    sendMsg(msg);
}

function onShowTextChanged(elem)
{
    var showText = elem.checked;

    var msg = {
        cmd: "ShowText",
        params: {
            show: showText
        }
    }

    sendMsg(msg);
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

    sendMsg(msg);
}

function onShowTrapOutlinesChanged(elem)
{
    var showTrapOutlines = elem.checked;

    var msg = {
        cmd: "ShowTrapOutlines",
        params: {
            show: showTrapOutlines
        }
    }

    sendMsg(msg);
}

function onShowFluidContactsChanged(elem)
{
    var showContacts = elem.checked;

    var msg = {
        cmd: "ShowFluidContacts",
        params: {
            show: showContacts
        }
    }

    sendMsg(msg);
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

    sendMsg(msg);
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

    sendMsg(msg);
}

function onButtonViewAllClicked()
{
    console.log("view all");

    var msg = {
        cmd: "ViewAll",
        params: {}
    }

    sendMsg(msg);
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

    sendMsg(msg);
}

function onColorScaleParamsChanged()
{
    var selectMapping = document.getElementById('selectColorScaleMapping');
    var selectRange   = document.getElementById('selectColorScaleRange');
    var editMinValue  = document.getElementById('editColorScaleMinValue');
    var editMaxValue  = document.getElementById('editColorScaleMaxValue');

    var mappings = [ "linear", "log" ];
    var ranges = ["auto", "manual" ];

    var msg = {
        cmd: "SetColorScaleParams",
        params: {
            mapping: mappings[selectMapping.selectedIndex],
            range: ranges[selectRange.selectedIndex],
            minval: parseFloat(editMinValue.value),
            maxval: parseFloat(editMaxValue.value)
        }
    }

    sendMsg(msg);
}

function onCellFilterToggled(elem)
{
    var msg = {
        cmd: "EnableCellFilter",
        params: {
            enabled: elem.checked
        }
    }

    sendMsg(msg);
}

function onCellFilterRangeChanged()
{
    var editMinValue = document.getElementById("editCellFilterMinValue");
    var editMaxValue = document.getElementById("editCellFilterMaxValue");

    var msg = {
        cmd: "SetCellFilterRange",
        params: {
            minval: parseFloat(editMinValue.value),
            maxval: parseFloat(editMaxValue.value)
        }
    }

    sendMsg(msg);
}

function onQualitySliderChanged(value)
{
    var msg = {
        cmd: "SetStillQuality",
        params: {
            quality: value
        }
    }

    sendMsg(msg);
}

function onInteractiveQualitySliderChanged(value)
{
    var msg = {
        cmd: "SetInteractiveQuality",
        params: {
            quality: value
        }
    }

    sendMsg(msg);
}

function onBandwidthSliderChanged(value)
{
    var msg = {
        cmd: "SetBandwidth",
        params: {
            bandwidth: value
        }
    }

    sendMsg(msg);
}

function onMaxFPSSliderChanged(value)
{
    var msg = {
        cmd: "SetMaxFPS",
        params: {
            maxFPS: value
        }
    }

    sendMsg(msg);
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
    	//console.log("latency: " + (newtimestamp - timestamp));
    	timestamp = 0;
    }
}

function logPickResult(pickResult)
{
    if(pickResult.type == "trap")
        console.log("Picked trap id = " + pickResult.trapID);
    else
        console.log("Picked " + pickResult.type + " \"" + pickResult.name 
            + "\" [" + pickResult.i + ", " + pickResult.j + ", " + pickResult.k + "] "
            + "property value " + pickResult.propertyValue);
}

function onFenceAdded(fenceId)
{
    var fencesDiv = document.getElementById("fences");
    var name = "Fence " + fenceId;

    fencesDiv.appendChild(
        createCheckBoxDiv(name, fenceId, true, onFenceCheckBoxChanged));
}

function handleEvent(e)
{
    if(e.type == "fenceAdded")
        onFenceAdded(e.params.fenceId);
}

function onSeismicSlicePositionChanged(index, elem)
{
    console.log("seismic slice " + index + " position = " + elem.value);

    var msg = {
        cmd: "SetSeismicSlicePosition",
        params: {
            index: index,
            position: elem.valueAsNumber
        }
    };

    sendMsg(msg);
}

function onSeismicSliceCheckBoxChanged(index, elem)
{
    console.log("seismic slice " + index + " enabled = " + elem.checked);

    var msg = {
        cmd: "EnableSeismicSlice",
        params: {
            index: index,
            enabled: elem.checked
        }
    }

    sendMsg(msg);
}

function receivedMessage(message)
{
    if(logMessages)
    	console.log(message);

    var msgObj = JSON.parse(message);
    if(msgObj.projectInfo)
    {
    	theRenderArea.projectInfo = msgObj.projectInfo;
        initUI(msgObj.projectInfo);
    }
    else if (msgObj.seismicInfo)
    {
        initSeismicUI(msgObj.seismicInfo);
    }
    else if (msgObj.pickResult) {
        logPickResult(msgObj.pickResult);
    }
    else if (msgObj.event) {
        handleEvent(msgObj.event);
    }
}

function measurebandwithandfps()
{
    bandwidthDiv.textContent = "Bandwidth: " + Math.round(dataSize / 1000) + "kB/s";
    fpsDiv.textContent = "FPS: " + fps;

    dataSize = 0;
    fps = 0;
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

function getSearchParameters() {
      var prmstr = window.location.search.substr(1);
      return prmstr != null && prmstr != "" ? transformToAssocArray(prmstr) : {};
}

function transformToAssocArray( prmstr ) {
    var params = {};
    var prmarr = prmstr.split("&");
    for ( var i = 0; i < prmarr.length; i++) {
        var tmparr = prmarr[i].split("=");
        params[tmparr[0]] = tmparr[1];
    }
    return params;
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

    var params = getSearchParameters();
    url += params.project;

    return url;
}

function init() 
{ 
    window.canvas = document.getElementById("TheCanvas");
    window.canvas.addEventListener("click", function(event)
    {
        var x = event.pageX - window.canvas.offsetLeft;
        var y = event.pageY - window.canvas.offsetTop;

        var msg = {
            cmd: "Pick",
            params: {
                x: x,
                y: y
            }
        }

        sendMsg(msg);
    });

    //$(window).resize(onWindowResize);

    // This function is called immediately after the page is loaded. Initialization of 
    // the renderArea. "TheCanvas" refers to the id of the canvas. 
    theRenderArea = new RemoteVizRenderArea(
        "TheCanvas",
    	window.innerWidth - leftMargin, 
    	window.innerHeight - bottomMargin);

    theRenderArea.addReceivedImageListener(receivedImage);
    theRenderArea.addMessageListener(receivedMessage);

    // Connects to the service. The IP address and the port refer to those of the service 
    // (see main.cpp). "Model" refers to the name of the requested renderArea.
    var url = websocketURL();// + generateGUID();
    theRenderArea.connectTo(url);

    bandwidthDiv = document.getElementById("bandwidthDiv");
    fpsDiv = document.getElementById("fpsDiv");

    // Calls a function or executes a code snippet repeatedly to refresh the bandwidth and the fps
    window.setInterval("measurebandwithandfps()",1000);
    //node = document.getElementById('bandwidthfps');
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

	   sendMsg(msg);
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
	   
        sendMsg(msg);
    }
}
