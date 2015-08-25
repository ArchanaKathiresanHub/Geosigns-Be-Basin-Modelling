function BSONParser(){this.offset=0;this.data=!1}BSONParser.prototype.readByte=function(){var b=this.data[this.offset];this.offset++;return b};BSONParser.prototype.readInt32=function(){var b;b=0|this.data[this.offset+3]<<24;b|=this.data[this.offset+2]<<16;b|=this.data[this.offset+1]<<8;b|=this.data[this.offset];this.offset+=4;return b};BSONParser.prototype.reset=function(b){this.length=this.offset=0;this.data=b};
BSONParser.prototype.AbToStr=function(b,c){for(var f="",l=0;l<c;l++)f+=String.fromCharCode(b[l]);return f};BSONParser.prototype.parse=function(b){if(!(b instanceof Uint8Array))throw"The input type must be a Uint8Array";this.reset(b);this.length=this.readInt32();this.length-=this.offset;return this.parseItem()};
BSONParser.prototype.parseItem=function(){for(var b={};this.offset<this.length-1;){var c=this.readByte();if(2==c){var c=this.parseKey(),f=this.parseString();b[c]=f}else if(5==c)c=this.parseKey(),f=this.parseBinary(),b[c]=f;else if(8==c)c=this.parseKey(),f=1==this.readByte(),b[c]=f;else if(10==c)c=this.parseKey(),b[c]=null;else if(16==c)c=this.parseKey(),f=this.readInt32(),b[c]=f;else{if(0==c)break;throw"Unrecognized data type 0x"+c.toString(16);}}return b};
BSONParser.prototype.parseKey=function(){for(var b="",c=0;256>c;c++){var f=this.readByte();if(0==f)break;b+=String.fromCharCode(f)}return b};BSONParser.prototype.parseString=function(){var b=this.readInt32(),c=new Uint8Array(this.data.buffer,this.offset,b);this.offset+=b;return this.AbToStr(c,b-1)};BSONParser.prototype.parseBinary=function(){var b=this.readInt32();this.offset++;var c=new Uint8Array(this.data.buffer,this.offset,b);this.offset+=b;return c};
function BSONEncoder(){this.buffer=[];this.offset=0}BSONEncoder.prototype.writeByte=function(b){this.buffer.push(b&255)};BSONEncoder.prototype.writeBytes=function(b){for(var c=b.byteLength,f=0;f<c;f++)this.writeByte(b[f])};BSONEncoder.prototype.writeInt32=function(b){this.buffer.push(b);this.buffer.push(b>>8);this.buffer.push(b>>16);this.buffer.push(b>>24)};BSONEncoder.prototype.writeKey=function(b){for(var c=b.length,f=0;f<c;f++)this.writeByte(b.charCodeAt(f));this.writeByte(0)};
BSONEncoder.prototype.writeString=function(b){var c=b.length;this.writeInt32(c+1);for(var f=0;f<c;f++)this.writeByte(b.charCodeAt(f));this.writeByte(0)};BSONEncoder.prototype.writeBinary=function(b){this.writeInt32(b.byteLength);this.writeByte(0);this.writeBytes(b)};BSONEncoder.prototype.pack=function(){var b=this.buffer.length,c=b+5,f=new Uint8Array(c),l=0;f[l++]=c>>0&255;f[l++]=c>>8&255;f[l++]=c>>16&255;f[l++]=c>>24&255;for(c=0;c<b;c++)f[l++]=this.buffer[c];f[l++]=0;return f};
BSONEncoder.prototype.encode=function(b){for(var c in b)this.encodeItem(c,b[c]);return this.pack()};
BSONEncoder.prototype.encodeItem=function(b,c){if("string"==typeof c)this.writeByte(2),this.writeKey(b),this.writeString(c);else if(c instanceof ArrayBuffer)this.writeByte(5),this.writeKey(b),this.writeBinary(c);else if(null==c||"undefined"==typeof c)this.writeByte(10),this.writeKey(b);else if("object"==typeof c)this.writeByte(3),this.writeKey(b),this.writeBytes((new BSONEncoder).encode(c));else if("number"==typeof c)this.writeByte(16),this.writeKey(b),this.writeInt32(c);else if("boolean"==typeof c)this.writeByte(8),
this.writeKey(b),!0==c?this.writeByte(1):this.writeByte(0);else throw"Unrecognized encode data type";};
function RemoteVizRenderArea(b,c,f,l){this.version=1;this.id=b;this.img=new Image;this.canvas=document.getElementById(this.id);this.ctx=this.canvas.getContext("2d");this.renderAreaHeight=this.renderAreaWidth=0;this.displayMode=l||"FIT";this.canvas.width=c;this.canvas.height=f;this.imageOriY=this.imageOriX=this.imageHeight=this.imageWidth=0;var k=this.clearCanvas=!1,n=!1;this.canvas.ondragstart=function(){return!1};this.events={};this.addServiceListener=function(a){this.events.hasOwnProperty("ServiceEvent")?
this.events.ServiceEvent.push(a):this.events.ServiceEvent=[a]};this.removeServiceListener=function(a){this.events.hasOwnProperty("ServiceEvent")&&(a=this.events.ServiceEvent.indexOf(a),-1!=a&&this.events.ServiceEvent.splice(a,1))};this.addReceivedImageListener=function(a){this.events.hasOwnProperty("ReceivedImageEvent")?this.events.ReceivedImageEvent.push(a):this.events.ReceivedImageEvent=[a]};this.removeReceivedImageListener=function(a){this.events.hasOwnProperty("ReceivedImageEvent")&&(a=this.events.ReceivedImageEvent.indexOf(a),
-1!=a&&this.events.ReceivedImageEvent.splice(a,1))};this.addResizeListener=function(a){this.events.hasOwnProperty("ResizeEvent")?this.events.ResizeEvent.push(a):this.events.ResizeEvent=[a]};this.removeResizeListener=function(a){this.events.hasOwnProperty("ResizeEvent")&&(a=this.events.ResizeEvent.indexOf(a),-1!=a&&this.events.ResizeEvent.splice(a,1))};this.addMessageListener=function(a){this.events.hasOwnProperty("CommandEvent")?this.events.CommandEvent.push(a):this.events.CommandEvent=[a]};this.removeMessageListener=
function(a){this.events.hasOwnProperty("CommandEvent")&&(a=this.events.CommandEvent.indexOf(a),-1!=a&&this.events.CommandEvent.splice(a,1))};this.addMouseMoveListener=function(a){this.events.hasOwnProperty("MouseMoveEvent")?this.events.MouseMoveEvent.push(a):this.events.MouseMoveEvent=[a]};this.removeMouseMoveListener=function(a){this.events.hasOwnProperty("MouseMoveEvent")&&(a=this.events.MouseMoveEvent.indexOf(a),-1!=a&&this.events.MouseMoveEvent.splice(a,1))};this.addMouseDownListener=function(a){this.events.hasOwnProperty("MouseDownEvent")?
this.events.MouseDownEvent.push(a):this.events.MouseDownEvent=[a]};this.removeMouseDownListener=function(a){this.events.hasOwnProperty("MouseDownEvent")&&(a=this.events.MouseDownEvent.indexOf(a),-1!=a&&this.events.MouseDownEvent.splice(a,1))};this.addMouseUpListener=function(a){this.events.hasOwnProperty("MouseUpEvent")?this.events.MouseUpEvent.push(a):this.events.MouseUpEvent=[a]};this.removeMouseUpListener=function(a){this.events.hasOwnProperty("MouseUpEvent")&&(a=this.events.MouseUpEvent.indexOf(a),
-1!=a&&this.events.MouseUpEvent.splice(a,1))};this.addMouseOverListener=function(a){this.events.hasOwnProperty("MouseOverEvent")?this.events.MouseOverEvent.push(a):this.events.MouseOverEvent=[a]};this.removeMouseOverListener=function(a){this.events.hasOwnProperty("MouseOverEvent")&&(a=this.events.MouseOverEvent.indexOf(a),-1!=a&&this.events.MouseOverEvent.splice(a,1))};this.addMouseOutListener=function(a){this.events.hasOwnProperty("MouseOutEvent")?this.events.MouseOutEvent.push(a):this.events.MouseOutEvent=
[a]};this.removeMouseOutListener=function(a){this.events.hasOwnProperty("MouseOutEvent")&&(a=this.events.MouseOutEvent.indexOf(a),-1!=a&&this.events.MouseOutEvent.splice(a,1))};this.addMouseWheelListener=function(a){this.events.hasOwnProperty("MouseWheelEvent")?this.events.MouseWheelEvent.push(a):this.events.MouseWheelEvent=[a]};this.removeMouseWheelListener=function(a){this.events.hasOwnProperty("MouseWheelEvent")&&(a=this.events.MouseWheelEvent.indexOf(a),-1!=a&&this.events.MouseWheelEvent.splice(a,
1))};this.addTouchStartListener=function(a){this.events.hasOwnProperty("TouchStartEvent")?this.events.TouchStartEvent.push(a):this.events.TouchStartEvent=[a]};this.removeTouchStartListener=function(a){this.events.hasOwnProperty("TouchStartEvent")&&(a=this.events.TouchStartEvent.indexOf(a),-1!=a&&this.events.TouchStartEvent.splice(a,1))};this.addTouchEndListener=function(a){this.events.hasOwnProperty("TouchEndEvent")?this.events.TouchEndEvent.push(a):this.events.TouchEndEvent=[a]};this.removeTouchEndListener=
function(a){this.events.hasOwnProperty("TouchEndEvent")&&(a=this.events.TouchEndEvent.indexOf(a),-1!=a&&this.events.TouchEndEvent.splice(a,1))};this.addTouchCancelListener=function(a){this.events.hasOwnProperty("TouchCancelEvent")?this.events.TouchCancelEvent.push(a):this.events.TouchCancelEvent=[a]};this.removeTouchCancelListener=function(a){this.events.hasOwnProperty("TouchCancelEvent")&&(a=this.events.TouchCancelEvent.indexOf(a),-1!=a&&this.events.TouchCancelEvent.splice(a,1))};this.addTouchMoveListener=
function(a){this.events.hasOwnProperty("TouchMoveEvent")?this.events.TouchMoveEvent.push(a):this.events.TouchMoveEvent=[a]};this.removeTouchMoveListener=function(a){this.events.hasOwnProperty("TouchMoveEvent")&&(a=this.events.TouchMoveEvent.indexOf(a),-1!=a&&this.events.TouchMoveEvent.splice(a,1))};this.addKeyUpListener=function(a){this.events.hasOwnProperty("KeyUpEvent")?this.events.KeyUpEvent.push(a):this.events.KeyUpEvent=[a]};this.removeKeyUpListener=function(a){this.events.hasOwnProperty("KeyUpEvent")&&
(a=this.events.KeyUpEvent.indexOf(a),-1!=a&&this.events.KeyUpEvent.splice(a,1))};this.addKeyDownListener=function(a){this.events.hasOwnProperty("KeyDownEvent")?this.events.KeyDownEvent.push(a):this.events.KeyDownEvent=[a]};this.removeKeyDownListener=function(a){this.events.hasOwnProperty("KeyDownEvent")&&(a=this.events.KeyDownEvent.indexOf(a),-1!=a&&this.events.KeyDownEvent.splice(a,1))};var d=function(a,g,m){if(!a.events.hasOwnProperty(m))return!0;a=a.events[m];m=a.length;for(var b=!1,c=0;c<m;c++)b|=
a[c](g);return b},x=function(a,g){if(a.events.hasOwnProperty("ReceivedImageEvent")){g&&g.length||(g=[]);for(var m=a.events.ReceivedImageEvent,b=m.length,c=0;c<b;c++)m[c].apply(null,g)}},r=function(a,g){if(a.events.hasOwnProperty("ServiceEvent")){g&&g.length||(g=[]);for(var m=a.events.ServiceEvent,b=m.length,c=0;c<b;c++)m[c].apply(null,g)}};!window.WebSocket&&window.MozWebSocket?window.WebSocket=window.MozWebSocket:window.WebSocket||window.MozWebSocket||alert("Your browser does not support WebSocket.");
var e=function(a,g){return function(){return g.apply(a,arguments)}};this.sendMessage=function(a){if(k&&null!=this.websocket){var g={type:"command"};g.message=a.toString();a=new BSONEncoder;this.websocket.send(a.encode(g))}};this.requestRenderAreaSize=function(a,g){if(k&&null!=this.websocket){var b={type:"requestsize"};b.width=parseInt(a);b.height=parseInt(g);var c=new BSONEncoder;this.websocket.send(c.encode(b))}};this.getContainerWidth=function(){return this.canvas.width};this.getContainerHeight=
function(){return this.canvas.height};this.getRenderAreaWidth=function(){return this.renderAreaWidth};this.getRenderAreaHeight=function(){return this.renderAreaHeight};this.resizeRenderAreaContainer=function(a,g){this.canvas.width=a;this.canvas.height=g;this.updateImageSizeAndPosition(!0);if(k&&null!=this.websocket){var b={type:"resizecontainer"};b.width=parseInt(a);b.height=parseInt(g);var c=new BSONEncoder;this.websocket.send(c.encode(b))}};var s=function(a,g){return Math.round((g-a.imageOriX)*
a.renderAreaWidth/a.imageWidth)},t=function(a,g){return Math.round((g-a.imageOriY)*a.renderAreaHeight/a.imageHeight)},p=function(a,g,b){"mousedown"==g?n=!0:"mouseup"==g&&(n=!1);var c=a.canvas.getBoundingClientRect(),d=s(a,b.clientX-c.left),f=t(a,a.canvas.height-(b.clientY-c.top));k&&null!=a.websocket&&(c={type:"mouseevent"},c.event=g,c.x=d,c.y=f,c.button=b.button,g=new BSONEncoder,a.websocket.send(g.encode(c)))},v=function(a,g,b){if(k&&null!=a.websocket){var c={type:"keyevent"};c.event=g;c.key=b.keyCode;
g=new BSONEncoder;a.websocket.send(g.encode(c))}},q=function(a,g,b){for(var c=b.changedTouches,d=0;d<c.length;d++){var f=c[d],e=a.canvas.getBoundingClientRect(),h=s(a,f.clientX-e.left),l=t(a,a.canvas.height-(f.clientY-e.top));k&&null!=a.websocket&&(e={type:"touchevent"},e.event=g,e.id=f.identifier,e.x=h,e.y=l,f=new BSONEncoder,a.websocket.send(f.encode(e)))}b.preventDefault()},h=function(a,b,c){"down"==b&&a.canvas.focus();if(c.pointerType==c.MSPOINTER_TYPE_MOUSE||"mouse"==c.pointerType)"up"==b?n=
!1:"down"==b&&(n=!0);var d=a.canvas.getBoundingClientRect(),f=s(a,c.clientX-d.left),e=t(a,a.canvas.height-(c.clientY-d.top));if(k&&null!=a.websocket){var h=b;if(c.pointerType==c.MSPOINTER_TYPE_MOUSE||"mouse"==c.pointerType)d={type:"mouseevent"},d.event="mouse"+b,d.x=f,d.y=e,d.button=0!=c.which?c.which-1:-1,b=new BSONEncoder,a.websocket.send(b.encode(d));else if(c.pointerType==c.MSPOINTER_TYPE_TOUCH||"touch"==c.pointerType){if("down"==b||"enter"==b)h="start";else if("up"==b||"cancel"==b||"leave"==
b)h="end";d={type:"touchevent"};d.event="touch"+h;d.x=f;d.y=e;d.id=c.pointerId;b=new BSONEncoder;a.websocket.send(b.encode(d))}}c.preventDefault()},u=function(a,b){if(k&&null!=a.websocket){var c={type:"mouseevent",event:"mousewheel"};c.delta=b.wheelDelta;var d=new BSONEncoder;a.websocket.send(d.encode(c))}};b=function(a){if(k){var b={type:"mouseevent",event:"mousewheel"};b.delta=120*-(a.detail/3);a=new BSONEncoder;this.websocket.send(a.encode(b))}};"undefined"!=typeof this.canvas.style.msTouchAction&&
(this.canvas.style.msTouchAction="none");"undefined"!=typeof this.canvas.style.TouchAction&&(this.canvas.style.TouchAction="none");this.updateImageSizeAndPosition=function(a){if("STRETCH"==this.displayMode)this.imageWidth=this.ctx.canvas.width,this.imageHeight=this.ctx.canvas.height,this.imageOriY=this.imageOriX=0;else if("CROP"==this.displayMode){var b=this.renderAreaWidth/this.ctx.canvas.width,c=this.renderAreaHeight/this.ctx.canvas.height;b>c?(this.imageWidth=Math.round(this.renderAreaWidth/c),
this.imageHeight=this.ctx.canvas.height,this.imageOriX=Math.round((this.ctx.canvas.width-this.imageWidth)/2),this.imageOriY=0):b<c?(this.imageWidth=this.ctx.canvas.width,this.imageHeight=Math.round(this.renderAreaHeight/b),this.imageOriX=0,this.imageOriY=Math.round((this.ctx.canvas.height-this.imageHeight)/2)):(this.imageWidth=this.ctx.canvas.width,this.imageHeight=this.ctx.canvas.height,this.imageOriY=this.imageOriX=0)}else b=this.renderAreaWidth/this.ctx.canvas.width,c=this.renderAreaHeight/this.ctx.canvas.height,
b>c?(this.imageWidth=this.ctx.canvas.width,this.imageHeight=Math.round(this.renderAreaHeight/b),this.imageOriX=0,this.imageOriY=Math.round((this.ctx.canvas.height-this.imageHeight)/2),this.clearCanvas=!0):b<c?(this.imageWidth=Math.round(this.renderAreaWidth/c),this.imageHeight=this.ctx.canvas.height,this.imageOriX=Math.round((this.ctx.canvas.width-this.imageWidth)/2),this.imageOriY=0,this.clearCanvas=!0):(this.imageWidth=this.ctx.canvas.width,this.imageHeight=this.ctx.canvas.height,this.imageOriY=
this.imageOriX=0);a&&this.redraw()};this.redraw=function(){this.clearCanvas&&(this.ctx.clearRect(0,0,this.ctx.canvas.width,this.ctx.canvas.height),this.clearCanvas=!1);this.ctx.drawImage(this.img,this.imageOriX,this.imageOriY,this.imageWidth,this.imageHeight)};this.img.onload=e(this,this.redraw);c=function(){var a=navigator.userAgent.toLowerCase();return-1!=a.indexOf("msie")?parseInt(a.split("msie")[1]):!1};window.navigator.msPointerEnabled||window.navigator.pointerEnabled?10==c()?(this.canvas.onmspointerdown=
e(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseDownEvent")||a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchStartEvent"))&&h(this,"down",a)}),this.canvas.onmspointermove=e(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseMoveEvent")||a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchMoveEvent"))&&h(this,"move",a)}),this.canvas.onmspointerup=e(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseUpEvent")||a.pointerType==
a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchEndEvent"))&&h(this,"up",a)}),this.canvas.onmspointercancel=e(this,function(a){a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchCancelEvent")&&h(this,"cancel",a)}),this.canvas.onmspointerout=e(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseOutEvent")||a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchEndEvent"))&&h(this,"leave",a)}),this.canvas.onmspointerover=e(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,
a,"MouseOverEvent")||a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchStartEvent"))&&h(this,"enter",a)}),this.canvas.onmousewheel=e(this,function(a){d(this,a,"MouseWheelEvent")&&u(this,a)}),b=e(this,function(a){n&&a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseUpEvent")&&h(this,"up",a)}),window.addEventListener("MSPointerUp",b,!1)):(this.canvas.onpointerdown=e(this,function(a){("mouse"==a.pointerType&&d(this,a,"MouseDownEvent")||"touch"==a.pointerType&&d(this,a,"TouchStartEvent"))&&h(this,
"down",a)}),this.canvas.onpointermove=e(this,function(a){("mouse"==a.pointerType&&d(this,a,"MouseMoveEvent")||"touch"==a.pointerType&&d(this,a,"TouchMoveEvent"))&&h(this,"move",a)}),this.canvas.onpointerup=e(this,function(a){("mouse"==a.pointerType&&d(this,a,"MouseUpEvent")||"touch"==a.pointerType&&d(this,a,"TouchEndEvent"))&&h(this,"up",a)}),this.canvas.onpointercancel=e(this,function(a){"touch"==a.pointerType&&d(this,a,"TouchCancelEvent")&&h(this,"cancel",a)}),this.canvas.onpointerout=e(this,function(a){("mouse"==
a.pointerType&&d(this,a,"MouseOutEvent")||"touch"==a.pointerType&&d(this,a,"TouchEndEvent"))&&h(this,"leave",a)}),this.canvas.onpointerover=e(this,function(a){("mouse"==a.pointerType&&d(this,a,"MouseOverEvent")||"touch"==a.pointerType&&d(this,a,"TouchStartEvent"))&&h(this,"enter",a)}),this.canvas.onmousewheel=e(this,function(a){d(this,a,"MouseWheelEvent")&&u(this,a)}),b=e(this,function(a){n&&"mouse"==a.pointerType&&d(this,a,"MouseUpEvent")&&h(this,"up",a)}),window.addEventListener("pointerup",b,!1)):
(this.canvas.ontouchstart=e(this,function(a){d(this,a,"TouchStartEvent")&&q(this,"touchstart",a)}),this.canvas.ontouchmove=e(this,function(a){d(this,a,"TouchMoveEvent")&&q(this,"touchmove",a)}),this.canvas.ontouchend=e(this,function(a){d(this,a,"TouchEndEvent")&&q(this,"touchend",a)}),this.canvas.ontouchcancel=e(this,function(a){d(this,a,"TouchCancelEvent")&&q(this,"touchend",a)}),this.canvas.onmousedown=e(this,function(a){d(this,a,"MouseDownEvent")&&p(this,"mousedown",a)}),this.canvas.onmouseover=
e(this,function(a){d(this,a,"MouseOverEvent")&&p(this,"mouseenter",a)}),this.canvas.onmouseout=e(this,function(a){d(this,a,"MouseOutEvent")&&p(this,"mouseleave",a)}),this.canvas.onmouseup=e(this,function(a){d(this,a,"MouseUpEvent")&&p(this,"mouseup",a)}),this.canvas.onmousemove=e(this,function(a){d(this,a,"MouseMoveEvent")&&p(this,"mousemove",a)}),this.canvas.onmousewheel=e(this,function(a){d(this,a,"MouseWheelEvent")&&u(this,a)}),b=b.bind(this),this.canvas.addEventListener("DOMMouseScroll",b,!1),
b=e(this,function(a){n&&d(this,a,"MouseUpEvent")&&p(this,"mouseup",a)}),window.addEventListener("mouseup",b,!1));this.canvas.onkeyup=e(this,function(a){d(this,a,"KeyUpEvent")&&v(this,"keyup",a)});this.canvas.onkeydown=e(this,function(a){d(this,a,"KeyDownEvent")&&v(this,"keydown",a)});this.canvas.setAttribute("tabindex","0");this.canvas.focus();var y=function(a){if(k&&null!=a.websocket){var b=new BSONEncoder;a.websocket.send(b.encode({type:"alive"}))}},w=function(){var a=(new Date).getTime();return"xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(/[xy]/g,
function(b){var c=(a+16*Math.random())%16|0;a=Math.floor(a/16);return("x"==b?c:c&7|8).toString(16)})},z=function(a){if(32768>a.length)return String.fromCharCode.apply(null,a);for(var b="",c=a.length,d=0;d<c;d+=32768)b+=String.fromCharCode.apply(null,a.subarray(d,d+32768));return b};this.wsonopen=function(){var a;var b=this.svcUrl;a=this.canvas.width;var c=this.canvas.height,d=b.match("^((ws|wss)://)([^/]*)")||[];a=0<d.length?-1==b.indexOf("requestedHeight")&&-1==b.indexOf("requestedWidth")?-1==b.indexOf("?")?
b.slice(d[0].length+1,b.length)+"?requestedWidth="+a+"&requestedHeight="+c:b.slice(d[0].length+1,b.length)+"&requestedWidth="+a+"&requestedHeight="+c:b.slice(d[0].length+1,b.length):null;null!=a?("undefined"!=typeof localStorage?localStorage.getItem("RemoteViz")?c=localStorage.getItem("RemoteViz"):(c=w(),localStorage.setItem("RemoteViz",c)):c=w(),b={type:"connection"},b.protocolversion=this.version,b.url=a,b.id=c,b.applicationname=window.location.host,b.containerwidth=this.canvas.width,b.containerheight=
this.canvas.height,a=new BSONEncoder,this.websocket.send(a.encode(b))):alert("Bad URL format.")};this.wsonclose=function(a){if(k){k=!1;var b=this;setTimeout(function(){r(b,[["disconnected","NETWORKFAILURE"]])},1)}};this.wsonmessage=function(a){if(null!=this.websocket){var b=this,c=(new BSONParser).parse(new Uint8Array(a.data));"accept"==c.type?(this.renderAreaWidth=c.width,this.renderAreaHeight=c.height,this.updateImageSizeAndPosition(!1),k=!0,setInterval(y,5E3,b),setTimeout(function(){r(b,[["connected",
c.width,c.height]])},1)):k&&"image"==c.type?(this.img.src="data:"+c.contenttype+";base64,"+window.btoa(z(c.data)),x(b,[c.data.length]),a=new BSONEncoder,this.websocket.send(a.encode({type:"ack"}))):k&&"command"==c.type?setTimeout(function(){var a=[c.message];if(b.events.hasOwnProperty("CommandEvent")){a&&a.length||(a=[]);for(var d=b.events.CommandEvent,f=d.length,e=0;e<f;e++)d[e].apply(null,a)}},1):k&&"resize"==c.type?(this.renderAreaWidth=c.width,this.renderAreaHeight=c.height,this.updateImageSizeAndPosition(!1),
setTimeout(function(){var a=[[c.width,c.height]];if(b.events.hasOwnProperty("ResizeEvent")){a&&a.length||(a=[]);for(var d=b.events.ResizeEvent,f=d.length,e=0;e<f;e++)d[e].apply(null,a)}},1)):"disconnect"==c.type&&setTimeout(function(){r(b,[["disconnected",c.reason]])},1)}};this.connectTo=function(a){this.svcUrl=a;this.websocket=new WebSocket(a);this.websocket.binaryType="arraybuffer";this.websocket.onopen=e(this,this.wsonopen);this.websocket.onclose=e(this,this.wsonclose);this.websocket.onmessage=
e(this,this.wsonmessage)};this.disconnect=function(){null!=this.websocket&&(this.websocket.close(),this.websocket=null,k=!1)};this.isConnected=function(){return null!=this.websocket?1==this.websocket.readyState:!1}};
