function Canvas4D(containerId, containerWidth, containerHeight, displayMode)
{
    var canvas = new RemoteVizRenderArea(
      containerId,
      containerWidth,
      containerHeight,
      displayMode);

    canvas.sendJSONMessage = function(msg)
    {
      var msgStr = JSON.stringify(msg, null, 4);

      if(logMessages)
        console.log(msgStr);

      this.sendMessage(msgStr);
    };

    canvas.pick = function(x, y)
    {
      var msg = {
          cmd: "Pick",
          params: {
              x: x,
              y: y
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableFormation = function(formationId, enable)
    {
      console.log("formation " + formationId + " enabled = " + enable);

      var msg = {
          cmd: "EnableFormation",
          params: {
              formationId: formationId,
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableAllFormations = function(enable)
    {
      var msg = {
          cmd: "EnableAllFormations",
          params: {
              enabled: enable
          }
      }

      this.sendJSONMessage(msg);
    };

    canvas.enableSurface = function(surfaceId, enable)
    {
      console.log("surface " + surfaceId + " enabled = " + enable);

      var msg = {
          cmd: "EnableSurface",
          params: {
              surfaceId: surfaceId,
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableAllSurfaces = function(enable)
    {
      var msg = {
          cmd: "EnableAllSurfaces",
          params: {
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableReservoir = function(reservoirId, enable)
    {
      console.log("reservoir " + reservoirId + " enabled = " + enable);

      var msg = {
          cmd: "EnableReservoir",
          params: {
              reservoirId: reservoirId,
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableAllReservoirs = function(enable)
    {
      var msg = {
          cmd: "EnableAllReservoirs",
          params: {
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableSlice = function(sliceId, enable)
    {
      console.log("sliceI enabled = " + enable);

      var msg = {
          cmd: "EnableSlice",
          params: {
              slice: sliceId,
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setSlicePosition = function(sliceId, position)
    {
      console.log("sliceI position = " + position);

      var msg = {
          cmd: "SetSlicePosition",
          params: {
              slice: sliceId,
              position: position
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableFault = function(faultId, enable)
    {
      console.log("fault " + faultId + " enabled = " + enable);

      var msg = {
          cmd: "EnableFault",
          params: {
              faultId: faultId,
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableAllFaults = function(enable)
    {
      var msg = {
          cmd: "EnableAllFaults",
          params: {
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableFlowLines = function(flowLinesId, enable)
    {
      var msg = {
          cmd: "EnableFlowLines",
          params: {
              flowLinesId: flowLinesId,
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableAllFlowLines = function(enable)
    {
      var msg = {
          cmd: "EnableAllFlowLines",
          params: {
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableFence = function(fenceId, enable)
    {
      console.log("fence " + fenceId + " enabled = " + enable);

      var msg = {
          cmd: "EnableFence",
          params: {
              fenceId: fenceId,
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setProperty = function(propertyId)
    {
      console.log("property " + propertyId + " clicked");

      var msg = {
          cmd: "SetProperty",
          params: {
              propertyId: propertyId
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setVerticalScale = function(scale)
    {
      console.log("vertical scale = " + scale);

      var msg = {
          cmd: "SetVerticalScale",
          params: {
              scale: scale
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setTransparency = function(transparency)
    {
        console.log("transparency = " + transparency);

        var msg = {
            cmd: "SetTransparency",
            params: {
                transparency: transparency
            }
        };

        this.sendJSONMessage(msg);
    };

    canvas.setRenderStyle = function(drawFaces, drawEdges)
    {
      var msg = {
          cmd: "SetRenderStyle",
          params: {
              drawFaces: drawFaces,
              drawEdges: drawEdges
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.showCoordinateGrid = function(show)
    {
      var msg = {
          cmd: "ShowCoordinateGrid",
          params: {
              show: show
          }
      }

      this.sendJSONMessage(msg);
    };

    canvas.showCompass = function(show)
    {
      var msg = {
          cmd: "ShowCompass",
          params: {
              show: show
          }
      }

      this.sendJSONMessage(msg);
    };

    canvas.showText = function(show)
    {
      var msg = {
          cmd: "ShowText",
          params: {
              show: show
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.showTraps = function(show)
    {
      var msg = {
          cmd: "ShowTraps",
          params: {
              show: show
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.showTrapOutlines = function(show)
    {
      var msg = {
          cmd: "ShowTrapOutlines",
          params: {
              show: show
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setFlowLinesStep = function(type, step)
    {
      var msg = {
          cmd: "SetFlowLinesStep",
          params: {
              type: type,
              step: step
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setFlowLinesThreshold = function(type, threshold)
    {
      var msg = {
          cmd: "SetFlowLinesThreshold",
          params: {
              type: type,
              threshold: threshold
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.showDrainageAreaOutline = function(type)
    {
      var msg = {
          cmd: "ShowDrainageAreaOutline",
          params: {
              type: type
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setProjection = function(projection)
    {
      var msg = {
          cmd: "SetProjection",
          params: {
              type: projection
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setCurrentSnapshot = function(snapshotId)
    {
      var msg = {
          cmd: "SetCurrentSnapshot",
          params: {
              index: snapshotId
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.viewAll = function()
    {
      console.log("view all");

      var msg = {
          cmd: "ViewAll",
          params: {}
      };

      this.sendJSONMessage(msg);
    };

    canvas.setViewPreset = function(preset)
    {
      var msg = {
          cmd: "SetViewPreset",
          params: {
              preset: preset
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setColorScaleParams = function(mapping, range, minVal, maxVal)
    {
      var msg = {
          cmd: "SetColorScaleParams",
          params: {
              mapping: mapping,
              range: range,
              minval: minVal,
              maxval: maxVal
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableCellFilter = function(enable)
    {
      var msg = {
          cmd: "EnableCellFilter",
          params: {
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setCellFilterRange = function(minValue, maxValue)
    {
      var msg = {
          cmd: "SetCellFilterRange",
          params: {
              minval: minValue,
              maxval: maxValue
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableSeismicSlice = function(sliceId, enable)
    {
      var msg = {
          cmd: "EnableSeismicSlice",
          params: {
              index: sliceId,
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setSeismicSlicePosition = function(sliceId, position)
    {
      var msg = {
          cmd: "SetSeismicSlicePosition",
          params: {
              index: sliceId,
              position: position
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.enableInterpolatedSurface = function(enable)
    {
      var msg = {
          cmd: "EnableInterpolatedSurface",
          params: {
              enabled: enable
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setInterpolatedSurfacePosition = function(position)
    {
      var msg = {
          cmd: "SetInterpolatedSurfacePosition",
          params: {
              position: position
          }
      };

      this.sendJSONMessage(msg);

    };

    canvas.setSeismicDataRange = function(minValue, maxValue)
    {
      var msg = {
          cmd: "SetSeismicDataRange",
          params: {
              minValue: minValue,
              maxValue: maxValue
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setStillQuality = function(quality)
    {
      var msg = {
          cmd: "SetStillQuality",
          params: {
              quality: quality
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setInteractiveQuality = function(quality)
    {
      var msg = {
          cmd: "SetInteractiveQuality",
          params: {
              quality: quality
          }
      };

      this.sendJSONMessage(msg);
    };

    canvas.setBandwidth = function(bandwidth)
    {
      var msg = {
          cmd: "SetBandwidth",
          params: {
              bandwidth: bandwidth
          }
        };

      this.sendJSONMessage(msg);
    };

    canvas.setMaxFPS = function(maxFPS)
    {
      var msg = {
          cmd: "SetMaxFPS",
          params: {
              maxFPS: maxFPS
          }
      };

      this.sendJSONMessage(msg);
    };

    return canvas;
};
