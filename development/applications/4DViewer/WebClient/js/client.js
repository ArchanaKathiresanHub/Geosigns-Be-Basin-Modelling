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

    uiElements.timeSlider.value = viewState.currentSnapshotIndex;

    checkPropertyRadioButton(viewState.currentPropertyId);

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
    uiElements.checkBoxPerspective.checked = viewState.showPerspective;

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

    uiElements.checkBoxSeismicSliceInline.checked = viewState.seismicInlineSliceEnabled;
    uiElements.checkBoxSeismicSliceCrossline.checked = viewState.seismicCrosslineSliceEnabled;
    uiElements.checkBoxInterpolatedSurface.checked = viewState.seismicInterpolatedSurfaceEnabled;
    uiElements.sliderSeismicSliceInline.value = viewState.seismicInlineSlicePosition;
    uiElements.sliderSeismicSliceCrossline.value = viewState.seismicCrosslineSlicePosition;
    uiElements.sliderInterpolatedSurface.value = viewState.seismicInterpolatedSurfacePosition;
    uiElements.editSeismicRangeMinValue.value = viewState.seismicDataRangeMinValue;
    uiElements.editSeismicRangeMaxValue.value = viewState.seismicDataRangeMaxValue;

    uiElements.sliderStillQuality.value = viewState.stillQuality;
    uiElements.sliderInteractiveQuality.value = viewState.interactiveQuality;
}

function onCheckBoxAllFormationsChanged(elem)
{
  theRenderArea.enableAllFormations(elem.checked);

    var formationsDiv = document.getElementById("formationsList");
    var checkBoxes = formationsDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onCheckBoxAllSurfacesChanged(elem)
{
  theRenderArea.enableAllSurfaces(elem.checked);

    var surfacesDiv = document.getElementById("surfacesList");
    var checkBoxes = surfacesDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onCheckBoxAllReservoirsChanged(elem)
{
  theRenderArea.enableAllReservoirs(elem.checked);

    var reservoirsDiv = document.getElementById("reservoirsList");
    var checkBoxes = reservoirsDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onCheckBoxAllFaultsChanged(elem)
{
  theRenderArea.enableAllFaults(elem.checked);

    var faultsDiv = document.getElementById("faultsList");
    var checkBoxes = faultsDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onCheckBoxAllFlowLinesChanged(elem)
{
  theRenderArea.enableAllFlowLines(elem.checked);

    var flowLinesDiv = document.getElementById("flowLinesList");
    var checkBoxes = flowLinesDiv.getElementsByTagName("input");
    for(var i=0; i < checkBoxes.length; ++i)
        checkBoxes[i].checked = elem.checked;
}

function onFormationCheckBoxChanged(elem, objectId)
{
  theRenderArea.enableFormation(objectId, elem.checked);
}

function onSurfaceCheckBoxChanged(elem, objectId)
{
  theRenderArea.enableSurface(objectId, elem.checked);
}

function onReservoirCheckBoxChanged(elem, objectId)
{
  theRenderArea.enableReservoir(objectId, elem.checked);
}

function onFaultCheckBoxChanged(elem, objectId)
{
  theRenderArea.enableFault(objectId, elem.checked);
}

function onFlowLinesCheckBoxChanged(elem, objectId)
{
  theRenderArea.enableFlowLines(objectId, elem.checked);
}

function onFenceCheckBoxChanged(elem, objectId)
{
  theRenderArea.enableFence(objectId, elem.checked);
}

function onPropertyRadioButtonClicked(elem, objectId)
{
  theRenderArea.setProperty(objectId);
}

function onFlowLinesStepSliderChanged(elem)
{
  var typeStr = (elem.id == "sliderFlowLinesLeakageStep")
      ? "FlowLinesLeakage"
      : "FlowLinesExpulsion";

  theRenderArea.setFlowLinesStep(typeStr, elem.valueAsNumber);
}

function onFlowLinesThresholdSliderChanged(elem)
{
    var typeStr = (elem.id == "sliderFlowLinesLeakageThreshold")
        ? "FlowLinesLeakage"
        : "FlowLinesExpulsion";
    var power = (elem.maxPower * elem.valueAsNumber) / elem.max;
    var threshold = Math.pow(10, power);

    theRenderArea.setFlowLinesThreshold(typeStr, threshold);
}

function onDrainageAreaRadioClicked(elem)
{
  theRenderArea.showDrainageAreaOutline(elem.value);
}

function onSlicePositionChanged(index, elem)
{
  theRenderArea.setSlicePosition(index, elem.valueAsNumber);
}

function onSliceCheckBoxChanged(index, elem)
{
  theRenderArea.enableSlice(index, elem.checked);
}

function onVerticalScaleSliderChanged(elem)
{
  theRenderArea.setVerticalScale(elem.valueAsNumber);
}

function onTransparencySliderChanged(elem)
{
    var transparency = elem.valueAsNumber / elem.max;
    theRenderArea.setTransparency(transparency);
}

function onRenderStyleChanged()
{
    var drawFaces = document.getElementById("checkBoxDrawFaces").checked;
    var drawEdges = document.getElementById("checkBoxDrawEdges").checked;
    theRenderArea.setRenderStyle(drawFaces, drawEdges);
}

function onShowGridChanged(elem)
{
  theRenderArea.showCoordinateGrid(elem.checked);
}

function onShowCompassChanged(elem)
{
  theRenderArea.showCompass(elem.checked);
}

function onShowTextChanged(elem)
{
  theRenderArea.showText(elem.checked);
}

function onShowTrapsChanged(elem)
{
  theRenderArea.showTraps(elem.checked);
}

function onShowTrapOutlinesChanged(elem)
{
  theRenderArea.showTrapOutlines(elem.checked);
}

function onPerspectiveChanged(elem)
{
    var projection = elem.checked
        ? "Perspective"
        : "Orthographic";
    theRenderArea.setProjection(projection);
}

function onTimeSliderChanged(elem)
{
  theRenderArea.setCurrentSnapshot(elem.valueAsNumber);
}

function onButtonViewAllClicked()
{
  theRenderArea.viewAll();
}

function onButtonViewPresetClicked(index)
{
  var viewDirs = [ "Top", "Left", "Front", "Bottom", "Right", "Back" ];
  theRenderArea.setViewPreset(viewDirs[index]);
}

function onColorScaleParamsChanged()
{
    var selectMapping = document.getElementById('selectColorScaleMapping');
    var selectRange   = document.getElementById('selectColorScaleRange');
    var editMinValue  = document.getElementById('editColorScaleMinValue');
    var editMaxValue  = document.getElementById('editColorScaleMaxValue');

    var mappings = [ "linear", "log" ];
    var ranges = ["auto", "manual" ];

    theRenderArea.setColorScaleParams(
      mappings[selectMapping.selectedIndex],
      ranges[selectRange.selectedIndex],
      parseFloat(editMinValue.value),
      parseFloat(editMaxValue.value));
}

function onCellFilterToggled(elem)
{
  theRenderArea.enableCellFilter(elem.checked);
}

function onCellFilterRangeChanged()
{
    var editMinValue = document.getElementById("editCellFilterMinValue");
    var editMaxValue = document.getElementById("editCellFilterMaxValue");

    theRenderArea.setCellFilterRange(
      parseFloat(editMinValue.value),
      parseFloat(editMaxValue.value));
}

function onQualitySliderChanged(value)
{
  theRenderArea.setStillQuality(value);
}

function onInteractiveQualitySliderChanged(value)
{
  theRenderArea.setInteractiveQuality(value);
}

function onBandwidthSliderChanged(value)
{
  theRenderArea.setBandwidth(value);
}

function onMaxFPSSliderChanged(value)
{
  theRenderArea.setMaxFPS(value);
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
    uiElements.checkBoxPerspective.checked = (type === "Perspective");
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
  theRenderArea.setSeismicSlicePosition(index, elem.valueAsNumber);
}

function onSeismicSliceCheckBoxChanged(index, elem)
{
  theRenderArea.enableSeismicSlice(index, elem.checked);
}

function onInterpolatedSurfaceCheckBoxChanged(elem)
{
  theRenderArea.enableInterpolatedSurface(elem.checked);
}

function onInterpolatedSurfacePositionChanged(elem)
{
  theRenderArea.setInterpolatedSurfacePosition(elem.valueAsNumber);
}

function onSeismicRangeChanged()
{
  var minValue = parseFloat(uiElements.editSeismicRangeMinValue.value);
  var maxValue = parseFloat(uiElements.editSeismicRangeMaxValue.value);

  theRenderArea.setSeismicDataRange(minValue, maxValue);
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
    else if(msgObj.viewstate)
    {
        initViewState(msgObj.viewstate)
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
    canvasDiv = document.getElementById("CanvasDiv");
    window.canvas = document.getElementById("TheCanvas");
    window.canvas.addEventListener("click", function(event)
    {
        var x = event.pageX - window.canvas.offsetLeft;
        var y = event.pageY - window.canvas.offsetTop;

        theRenderArea.pick(x, y);
    });

    window.onresize = resizeCanvas;//onWindowResize;

    var containerWidth  = canvasDiv.clientWidth;
    var containerHeight = canvasDiv.clientHeight;
    var url = websocketURL();// + generateGUID();

    // This function is called immediately after the page is loaded. Initialization of
    // the renderArea. "TheCanvas" refers to the id of the canvas.
    //theRenderArea = new RemoteVizRenderArea("TheCanvas", containerWidth, containerHeight);
    theRenderArea = new Canvas4D("TheCanvas", containerWidth, containerHeight);
    theRenderArea.addReceivedImageListener(receivedImage);
    theRenderArea.addMessageListener(receivedMessage);
    theRenderArea.connectTo(url);

    bandwidthDiv = document.getElementById("bandwidthDiv");
    fpsDiv = document.getElementById("fpsDiv");

    // Calls a function or executes a code snippet repeatedly to refresh the bandwidth and the fps
    window.setInterval("measurebandwithandfps()",1000);
}
