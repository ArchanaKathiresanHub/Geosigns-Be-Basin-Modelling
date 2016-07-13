var theRenderArea = null;
var canvasDiv = null;
var bandwidthDiv = null;
var fpsDiv = null;
var dataSize = 0;
var fps = 0;
var resizeTimer = null;
var timestamp = 0;
var uiElements = null;
var logMessages = true;

function collectUIElements()
{
    var formationsDiv = document.getElementById("formationsList");
    var formationCheckBoxes = formationsDiv.getElementsByTagName("input");

    var surfacesDiv = document.getElementById("surfacesList");
    var surfaceCheckBoxes = surfacesDiv.getElementsByTagName("input");

    var reservoirsDiv = document.getElementById("reservoirsList");
    var reservoirCheckBoxes = reservoirsDiv.getElementsByTagName("input");

    var faultsDiv = document.getElementById("faultsList");
    var faultsCheckBoxes = faultsDiv.getElementsByTagName("input");

    var flowLinesDiv = document.getElementById("flowLinesList");
    var flowLinesCheckBoxes = flowLinesDiv.getElementsByTagName("input");

    var propertiesDiv = document.getElementById("propertiesList");
    var propertyRadioButtons = propertiesDiv.getElementsByTagName("input");

    uiElements = {
        timeSlider: document.getElementById("timeSlider"),
        formationCheckBoxes: formationCheckBoxes,
        surfaceCheckBoxes: surfaceCheckBoxes,
        reservoirCheckBoxes: reservoirCheckBoxes,
        faultsCheckBoxes: faultsCheckBoxes,
        flowLinesCheckBoxes: flowLinesCheckBoxes,
        propertyRadioButtons: propertyRadioButtons,
        checkBoxAllFormations: document.getElementById("checkBoxAllFormations"),
        checkBoxAllSurfaces: document.getElementById("checkBoxAllSurfaces"),
        checkBoxAllReservoirs: document.getElementById("checkBoxAllReservoirs"),
        checkBoxAllFaults: document.getElementById("checkBoxAllFaults"),
        checkBoxAllFlowLines: document.getElementById("checkBoxAllFlowLines"),
        checkBoxDrawTraps: document.getElementById("checkBoxDrawTraps"),
        checkBoxDrawTrapOutlines: document.getElementById("checkBoxDrawTrapOutlines"),
        checkBoxDrawFluidContacts: document.getElementById("checkBoxDrawFluidContacts"),
        radioButtonDrainageAreaNone: document.getElementById("radioButtonDrainageAreaNone"),
        radioButtonDrainageAreaFluid: document.getElementById("radioButtonDrainageAreaFluid"),
        radioButtonDrainageAreaGas: document.getElementById("radioButtonDrainageAreaGas"),
        sliderFlowLinesLeakageStep: document.getElementById("sliderFlowLinesLeakageStep"),
        sliderFlowLinesLeakageThreshold: document.getElementById("sliderFlowLinesLeakageThreshold"),
        sliderFlowLinesExpulsionStep: document.getElementById("sliderFlowLinesExpulsionStep"),
        sliderFlowLinesExpulsionThreshold: document.getElementById("sliderFlowLinesExpulsionThreshold"),
        sliderVerticalScale: document.getElementById("sliderVerticalScale"),
        sliderTransparency: document.getElementById("sliderTransparency"),
        checkBoxDrawFaces: document.getElementById("checkBoxDrawFaces"),
        checkBoxDrawEdges: document.getElementById("checkBoxDrawEdges"),
        checkBoxDrawGrid: document.getElementById("checkBoxDrawGrid"),
        checkBoxDrawCompass: document.getElementById("checkBoxDrawCompass"),
        checkBoxDrawText: document.getElementById("checkBoxDrawText"),
        checkBoxPerspective: document.getElementById("checkBoxPerspective"),
        sliceICheckBox: document.getElementById("sliceICheckBox"),
        sliceJCheckBox: document.getElementById("sliceJCheckBox"),
        sliceISlider: document.getElementById("sliceISlider"),
        sliceJSlider: document.getElementById("sliceJSlider"),
        fenceCheckBoxes: {},
        selectColorScaleMapping: document.getElementById('selectColorScaleMapping'),
        selectColorScaleRange: document.getElementById('selectColorScaleRange'),
        editColorScaleMinValue: document.getElementById('editColorScaleMinValue'),
        editColorScaleMaxValue: document.getElementById('editColorScaleMaxValue'),
        checkBoxCellFilter: document.getElementById("checkBoxCellFilter"),
        editCellFilterMinValue: document.getElementById("editCellFilterMinValue"),
        editCellFilterMaxValue: document.getElementById("editCellFilterMaxValue"),
        checkBoxSeismicSliceInline: document.getElementById("sliceInlineCheckBox"),
        checkBoxSeismicSliceCrossline: document.getElementById("sliceCrosslineCheckBox"),
        checkBoxInterpolatedSurface: document.getElementById("interpolatedSurfaceCheckBox"),
        sliderSeismicSliceInline: document.getElementById("sliceInlineSlider"),
        sliderSeismicSliceCrossline: document.getElementById("sliceCrosslineSlider"),
        sliderInterpolatedSurface: document.getElementById("interpolatedSurfaceSlider"),
        editSeismicRangeMinValue: document.getElementById("seismicRangeMinValue"),
        editSeismicRangeMaxValue: document.getElementById("seismicRangeMaxValue"),
        sliderStillQuality: document.getElementById("sliderStillQuality"),
        sliderInteractiveQuality: document.getElementById("sliderInteractiveQuality"),
        sliderMaxFPS: document.getElementById("sliderMaxFPS")
    };
}

function checkPropertyRadioButton(propertyId)
{
  // check radio button for current property
  if(propertyId >= 0)
  {
    var derivedPropertyBaseId = 65536;
    var index = propertyId;
    if(propertyId >= derivedPropertyBaseId)
    {
      var derivedPropertyIndex = propertyId - derivedPropertyBaseId;
      index = uiElements.propertyRadioButtons.length - 3 + derivedPropertyIndex;
    }

    uiElements.propertyRadioButtons[index].checked = true;
  }
}

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

function createCheckBoxDiv(name, checkBoxId, objectId, checked, changedHandler)
{
    var cb = document.createElement("input");
    cb.type="checkbox";
    cb.id = checkBoxId;
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
        formationsDiv.appendChild(createCheckBoxDiv(names[i], "formation_" + i, i, true, onFormationCheckBoxChanged));
}

function initSurfaces(names)
{
    var surfacesDiv = document.getElementById("surfacesList");

    for(var i=0; i < names.length; ++i)
        surfacesDiv.appendChild(createCheckBoxDiv(names[i], "surface_" + i, i, false, onSurfaceCheckBoxChanged));
}

function initReservoirs(names)
{
    var reservoirsDiv = document.getElementById("reservoirsList");

    for(var i=0; i < names.length; ++i)
        reservoirsDiv.appendChild(createCheckBoxDiv(names[i], "reservoir_" + i, i, false, onReservoirCheckBoxChanged));
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
            collDiv.appendChild(createCheckBoxDiv(collection.faults[j], "fault_" + objectId, objectId, false, onFaultCheckBoxChanged));

        objectId++;

        collectionsDiv.appendChild(collDiv);
    }
}

function initFlowLines(names)
{
    var flowLinesDiv = document.getElementById("flowLinesList");

    for(var i=0; i < names.length; ++i)
        flowLinesDiv.appendChild(createCheckBoxDiv(names[i], "flowlines_" + i, i, false, onFlowLinesCheckBoxChanged));
}

function initProperties(names)
{
    //var fmt = "<div><input type='radio' name='property' value='xxx' onclick='onPropertyRadioButtonClicked(this)'>xxx</input></div>";
    var propertiesList = document.getElementById("propertiesList");

    //for(var i=0; i < names.length; ++i)
    //    propertiesList.innerHTML += fmt.replace(/xxx/g, names[i]);

    for(var i=0; i < names.length; ++i)
        propertiesList.appendChild(createRadioButtonDiv("properties", names[i], i, onPropertyRadioButtonClicked));

    var derivedPropertyBaseId = 65536;
    var derivedProperties =
    [
      "FormationId",
      "PersistentTrapId",
      "FluidContacts"
    ];

    for(var i=0; i < derivedProperties.length; ++i)
      propertiesList.appendChild(
        createRadioButtonDiv(
          "properties",
          derivedProperties[i],
          derivedPropertyBaseId + i,
          onPropertyRadioButtonClicked));
}

function initUI(projectInfo)
{
    initFormations(projectInfo.formations);
    initSurfaces(projectInfo.surfaces);
    initReservoirs(projectInfo.reservoirs);
    initFaults(projectInfo.faultCollections);
    initFlowLines(projectInfo.flowLines);
    initProperties(projectInfo.properties);

    collectUIElements();

    var sliceI = uiElements.sliceISlider;// document.getElementById("sliceISlider");
    sliceI.min = 0;
    sliceI.max = projectInfo.numI - 2;
    sliceI.step = 1;

    var sliceJ = uiElements.sliceJSlider;// document.getElementById("sliceJSlider");
    sliceJ.min = 0;
    sliceJ.max = projectInfo.numJ - 2;
    sliceJ.step = 1;

    var timeSlider = document.getElementById("timeSlider");
    timeSlider.min = 0;
    timeSlider.max = projectInfo.snapshotCount - 1;
    timeSlider.step = 1;
    timeSlider.value = 0;

    // seismic interpolated surface slider
    var surfaceSlider = document.getElementById("interpolatedSurfaceSlider");
    surfaceSlider.min = 0.0;
    surfaceSlider.max = projectInfo.numK;

    uiElements.sliderFlowLinesLeakageThreshold.maxPower = 9.0;
    uiElements.sliderFlowLinesExpulsionThreshold.maxPower = 9.0;
    // Init JPEG quality
    onQualitySliderChanged(uiElements.sliderStillQuality.valueAsNumber);
    onInteractiveQualitySliderChanged(uiElements.sliderInteractiveQuality.valueAsNumber);
    // Init H264 quality
    //onBandwidthSliderChanged(document.getElementById("bandwidthSlider").valueAsNumber * 8192);
    onMaxFPSSliderChanged(uiElements.sliderMaxFPS.valueAsNumber);
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

function setExponentialSliderValue(elem, value)
{
  elem.value = elem.max * Math.log10(value) / elem.maxPower;
}

function initViewState(viewState)
{
    for(var i=0; i < uiElements.formationCheckBoxes.length; ++i)
        uiElements.formationCheckBoxes[i].checked = viewState.formationVisibility[i];

    for(var i=0; i < uiElements.surfaceCheckBoxes.length; ++i)
        uiElements.surfaceCheckBoxes[i].checked = viewState.surfaceVisibility[i];

    for(var i=0; i < uiElements.reservoirCheckBoxes.length; ++i)
        uiElements.reservoirCheckBoxes[i].checked = viewState.reservoirVisibility[i];

    uiElements.timeSlider.value = viewState.snapshotIndex;

    checkPropertyRadioButton(viewState.propertyId);

    uiElements.checkBoxDrawTraps.checked = viewState.showTraps;
    uiElements.checkBoxDrawTrapOutlines.checked = viewState.showTrapOutlines;

    switch(viewState.drainageAreaType)
    {
      case 0: uiElements.radioButtonDrainageAreaNone.checked = true; break;
      case 1: uiElements.radioButtonDrainageAreaFluid.checked = true; break;
      case 2: uiElements.radioButtonDrainageAreaGas.checked = true; break;
    }

    uiElements.sliderFlowLinesExpulsionStep.value = viewState.flowLinesExpulsionStep;
    setExponentialSliderValue(uiElements.sliderFlowLinesExpulsionThreshold, viewState.flowLinesExpulsionThreshold);
    uiElements.sliderFlowLinesLeakageStep.value = viewState.flowLinesLeakageStep;
    setExponentialSliderValue(uiElements.sliderFlowLinesLeakageThreshold, viewState.flowLinesLeakageThreshold);

    uiElements.sliderVerticalScale.value = viewState.verticalScale;
    uiElements.sliderTransparency.value = viewState.transparency * uiElements.sliderTransparency.max;

    uiElements.checkBoxDrawFaces.checked = viewState.showFaces;
    uiElements.checkBoxDrawEdges.checked = viewState.showEdges;
    uiElements.checkBoxDrawGrid.checked = viewState.showGrid;
    uiElements.checkBoxDrawCompass.checked = viewState.showCompass;
    uiElements.checkBoxDrawText.checked = viewState.showText;

    uiElements.sliceICheckBox.checked = viewState.sliceEnabled[0];
    uiElements.sliceJCheckBox.checked = viewState.sliceEnabled[1];
    uiElements.sliceISlider.value = viewState.slicePosition[0];
    uiElements.sliceJSlider.value = viewState.slicePosition[1];

    for(var i=0; i < viewState.fences.length; ++i)
      addFenceEntry(viewState.fences[i].id, viewState.fences[i].visible);

    uiElements.selectColorScaleMapping.selectedIndex = viewState.colorScaleMapping;
    uiElements.selectColorScaleRange.selectedIndex = viewState.colorScaleRange;
    uiElements.editColorScaleMinValue.value = viewState.colorScaleMinValue;
    uiElements.editColorScaleMaxValue.value = viewState.colorScaleMaxValue;

    uiElements.checkBoxCellFilter.checked = viewState.cellFilterEnabled;
    uiElements.editCellFilterMinValue.value = viewState.cellFilterMinValue;
    uiElements.editCellFilterMaxValue.value = viewState.cellFilterMaxValue;
}

function initSeismicState(seismicState)
{
    uiElements.checkBoxSeismicSliceInline.checked = seismicState.seismicInlineSliceEnabled;
    uiElements.checkBoxSeismicSliceCrossline.checked = seismicState.seismicCrosslineSliceEnabled;
    uiElements.checkBoxInterpolatedSurface.checked = seismicState.seismicInterpolatedSurfaceEnabled;
    uiElements.sliderSeismicSliceInline.value = seismicState.seismicInlineSlicePosition;
    uiElements.sliderSeismicSliceCrossline.value = seismicState.seismicCrosslineSlicePosition;
    uiElements.sliderInterpolatedSurface.value = seismicState.seismicInterpolatedSurfacePosition;
    uiElements.editSeismicRangeMinValue.value = seismicState.seismicDataRangeMinValue;
    uiElements.editSeismicRangeMaxValue.value = seismicState.seismicDataRangeMaxValue;
}

function initAreaState(areaState)
{
    uiElements.sliderStillQuality.value = areaState.stillQuality;
    uiElements.sliderInteractiveQuality.value = areaState.interactiveQuality;
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

    var power = (elem.maxPower * elem.valueAsNumber) / elem.max;
    var thresholdVal = Math.pow(10, power);

    var msg = {
        cmd: "SetFlowLinesThreshold",
        params: {
            type: typeStr,
            threshold: thresholdVal
        }
    }

    sendMsg(msg);
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
    var w = canvasDiv.clientWidth;
    var h = canvasDiv.clientHeight;
    //theRenderArea.requestRenderAreaSize(w, h);
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

function addFenceEntry(fenceId, visible)
{
  var fencesDiv = document.getElementById("fences");
  var name = "Fence " + fenceId;
  var id = "fence_" + fenceId;
  var div = createCheckBoxDiv(name, id, fenceId, visible, onFenceCheckBoxChanged);
  fencesDiv.appendChild(div);

  // retrieve the checkbox inside the div
  uiElements.fenceCheckBoxes[fenceId] = document.getElementById(id);
}

// -----------------------------------------------------------------------------
// Events
// -----------------------------------------------------------------------------
var eventHandler = {

  fenceAdded: function(params)
  {
    addFenceEntry(params.fenceId, true);
  },

    pointPicked: function(params)
{
    logPickResult(params.pickResult);
},

    projectLoaded: function(params)
{
    theRenderArea.projectInfo = params.projectInfo;
    initUI(params.projectInfo);

    if(params.seismicInfo)
{
    initSeismicUI(params.seismicInfo);
    initSeismicState(params.seismicState);
}
    initViewState(params.viewState);
    initAreaState(params.areaState);

    uiElements.checkBoxPerspective.checked = (params.projection === "perspective");
},

  connectionCountChanged: function(params)
  {
      var title = "BPA 4D Viewer";

      if(params.count > 1)
          title += " (" + params.count + ")";

      document.title = title;
   },

  formationEnabled: function(params)
  {
    uiElements.formationCheckBoxes[params.formationId].checked = params.enabled;
  },

  allFormationsEnabled: function(params)
  {
    for(var i=0; i < uiElements.formationCheckBoxes.length; ++i)
      uiElements.formationCheckBoxes[i].checked = params.enabled;

    uiElements.checkBoxAllFormations.checked = params.enabled;
  },

  surfaceEnabled: function(params)
  {
    uiElements.surfaceCheckBoxes[params.surfaceId].checked = params.enabled;
  },

  allSurfacesEnabled: function(params)
  {
    for(var i=0; i < uiElements.surfaceCheckBoxes.length; ++i)
      uiElements.surfaceCheckBoxes[i].checked = params.enabled;

    uiElements.checkBoxAllSurfaces.checked = params.enabled;
  },

  reservoirEnabled: function(params)
  {
    uiElements.reservoirCheckBoxes[params.reservoirId].checked = params.enabled;
  },

  allReservoirsEnabled: function(params)
  {
    for(var i=0; i < uiElements.reservoirCheckBoxes.length; ++i)
      uiElements.reservoirCheckBoxes[i].checked = params.enabled;

    uiElements.checkBoxAllReservoirs.checked = params.enabled;
  },

  faultEnabled: function(params)
  {
    uiElements.faultCheckBoxes[params.faultId].checked = params.enabled;
  },

  allFaultsEnabled: function(params)
  {
    for(var i=0; i < uiElements.faultCheckBoxes.length; ++i)
      uiElements.faultCheckBoxes[i].checked = params.enabled;

    uiElements.checkBoxAllFaults.checked = params.enabled;
  },

  flowLinesEnabled: function(params)
  {
    uiElements.flowLinesCheckBoxes[params.flowLinesId].checked = params.enabled;
  },

  allFlowLinesEnabled: function(params)
  {
    for(var i=0; i < uiElements.flowLinesCheckBoxes.length; ++i)
      uiElements.flowLinesCheckBoxes[i].checked = params.enabled;

    uiElements.checkBoxAllFlowLines.checked = params.enabled;
  },

  sliceEnabled: function(params)
  {
    var checkboxes = [
      uiElements.sliceICheckBox,
      uiElements.sliceJCheckBox
    ];

    checkboxes[params.slice].checked = params.enabled;
  },

  slicePositionChanged: function(params)
  {
    var sliders = [
      uiElements.sliceISlider,
      uiElements.sliceJSlider
    ];

    sliders[params.slice].value = params.position;
  },

  fenceEnabled: function(params)
  {
    uiElements.fenceCheckBoxes[params.fenceId].checked = params.enabled;
  },

  currentPropertyChanged: function(params)
  {
    checkPropertyRadioButton(params.propertyId);
  },

  verticalScaleChanged: function(params)
  {
    uiElements.sliderVerticalScale.value = params.scale;
  },

  transparencyChanged: function(params)
  {
    uiElements.sliderTransparency.value =
      params.transparency * uiElements.sliderTransparency.max;
  },

  renderStyleChanged: function(params)
  {
    uiElements.checkBoxDrawFaces.checked = params.drawFaces;
    uiElements.checkBoxDrawEdges.checked = params.drawEdges;
  },

  coordinateGridEnabled: function(params)
  {
    uiElements.checkBoxDrawGrid.checked = params.show;
  },

  compassEnabled: function(params)
  {
    uiElements.checkBoxDrawCompass.checked = params.show;
  },

  textEnabled: function(params)
  {
    uiElements.checkBoxDrawText.checked = params.show;
  },

  trapsEnabled: function(params)
  {
    uiElements.checkBoxDrawTraps.checked = params.show;
  },

  trapOutlinesEnabled: function(params)
  {
    uiElements.checkBoxDrawTrapOutlines.checked = params.show;
  },

  flowLinesStepChanged: function(params)
  {
    var slider = (params.type === "FlowLinesExpulsion")
      ? uiElements.sliderFlowLinesExpulsionStep
      : uiElements.sliderFlowLinesLeakageStep;

    slider.value = params.step;
  },

  flowLinesThresholdChanged: function(params)
  {
    var slider = (params.type === "FlowLinesExpulsion")
      ? uiElements.sliderFlowLinesExpulsionThreshold
      : uiElements.sliderFlowLinesLeakageThreshold;

    setExponentialSliderValue(slider, params.threshold);
  },

  drainageAreaOutlinesEnabled: function(params)
  {
    if(params.type === "DrainageAreaFluid")
      uiElements.radioButtonDrainageAreaFluid.checked = true;
    else if(params.type === "DrainageAreaGas")
      uiElements.radioButtonDrainageAreaGas.checked = true;
    else
      uiElements.radioButtonDrainageAreaNone.checked = true;
  },

  projectionChanged: function(params)
  {
    uiElements.checkBoxPerspective.checked = (params.type === "Perspective");
  },

  currentSnapshotChanged: function(params)
  {
    uiElements.timeSlider.value = params.index;
  },

  colorScaleParamsChanged: function(params)
  {
    uiElements.selectColorScaleMapping.selectedIndex =
      (params.mapping == "linear") ? 0 : 1;
    uiElements.selectColorScaleRange.selectedIndex =
      (params.range == "auto") ? 0 : 1;

    uiElements.editColorScaleMinValue.value = params.minval;
    uiElements.editColorScaleMaxValue.value = params.maxval;
  },

  cellFilterEnabled: function(params)
  {
    uiElements.checkBoxCellFilter.checked = params.enabled;
  },

  cellFilterRangeChanged: function(params)
  {
    uiElements.editCellFilterMinValue.value = params.minval;
    uiElements.editCellFilterMaxValue.value = params.maxval;
  },

  seismicSliceEnabled: function(params)
  {
    var checkboxes = [
      uiElements.checkBoxSeismicSliceInline,
      uiElements.checkBoxSeismicSliceCrossline
    ];

    checkboxes[params.index].checked = params.enabled;
  },

  seismicSlicePositionChanged: function(params)
  {
    var sliders = [
      uiElements.sliderSeismicSliceInline,
      uiElements.sliderSeismicSliceCrossline
    ];

    sliders[params.index].value = params.position;
  },

  interpolatedSurfaceEnabled: function(params)
  {
    uiElements.checkBoxInterpolatedSurface.checked = params.enabled;
  },

  interpolatedSurfacePositionChanged: function(params)
  {
    uiElements.sliderInterpolatedSurface.value = params.position;
  },

  seismicDataRangeChanged: function(params)
  {
    uiElements.editSeismicRangeMinValue.value = params.minValue;
    uiElements.editSeismicRangeMaxValue.value = params.maxValue;
  },

  stillQualityChanged: function(params)
  {
    uiElements.sliderStillQuality.value = params.quality;
  },

  interactiveQualityChanged: function(params)
  {
    uiElements.sliderInteractiveQuality.value = params.quality;
  }
};

function handleEvent(e)
{
  if(eventHandler[e.type])
    eventHandler[e.type](e.params);
  else
    console.log("unknown event: " + e.type);
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

function onInterpolatedSurfaceCheckBoxChanged(elem)
{
    var msg = {
        cmd: "EnableInterpolatedSurface",
        params: {
            enabled: elem.checked
        }
    }

    sendMsg(msg);
}

function onInterpolatedSurfacePositionChanged(elem)
{
    var msg = {
        cmd: "SetInterpolatedSurfacePosition",
        params: {
            position: elem.valueAsNumber
        }
    }

    sendMsg(msg);
}

function onSeismicRangeChanged()
{
    var minValue = parseFloat(uiElements.editSeismicRangeMinValue.value);
    var maxValue = parseFloat(uiElements.editSeismicRangeMaxValue.value);

    var msg = {
        cmd: "SetSeismicDataRange",
        params: {
            minValue: minValue,
            maxValue: maxValue
        }
    }

    sendMsg(msg);
}

function receivedMessage(message)
{
    if(logMessages)
    	console.log(message);

    var msgObj = JSON.parse(message);
    if (msgObj.event) {
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
    canvasDiv = document.getElementById("CanvasDiv");
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

    window.onresize = resizeCanvas;//onWindowResize;

    var containerWidth  = canvasDiv.clientWidth;
    var containerHeight = canvasDiv.clientHeight;

    // This function is called immediately after the page is loaded. Initialization of
    // the renderArea. "TheCanvas" refers to the id of the canvas.
    theRenderArea = new RemoteVizRenderArea("TheCanvas", containerWidth, containerHeight);
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
}
