function BSONParser(){this.offset=0;this.data=!1}BSONParser.prototype.readByte=function(){var b=this.data[this.offset];this.offset++;return b};BSONParser.prototype.readInt32=function(){var b;b=0|this.data[this.offset+3]<<24;b|=this.data[this.offset+2]<<16;b|=this.data[this.offset+1]<<8;b|=this.data[this.offset];this.offset+=4;return b};BSONParser.prototype.reset=function(b){this.length=this.offset=0;this.data=b};
BSONParser.prototype.AbToStr=function(b,c){for(var f="",p=0;p<c;p++)f+=String.fromCharCode(b[p]);return f};BSONParser.prototype.parse=function(b){if(!(b instanceof Uint8Array))throw"The input type must be a Uint8Array";this.reset(b);this.length=this.readInt32();this.length-=this.offset;return this.parseItem()};
BSONParser.prototype.parseItem=function(){for(var b={};this.offset<this.length-1;){var c=this.readByte();if(2==c){var c=this.parseKey(),f=this.parseString();b[c]=f}else if(5==c)c=this.parseKey(),f=this.parseBinary(),b[c]=f;else if(8==c)c=this.parseKey(),f=1==this.readByte(),b[c]=f;else if(10==c)c=this.parseKey(),b[c]=null;else if(16==c)c=this.parseKey(),f=this.readInt32(),b[c]=f;else{if(0==c)break;throw"Unrecognized data type 0x"+c.toString(16);}}return b};
BSONParser.prototype.parseKey=function(){for(var b="",c=0;256>c;c++){var f=this.readByte();if(0==f)break;b+=String.fromCharCode(f)}return b};BSONParser.prototype.parseString=function(){var b=this.readInt32(),c=new Uint8Array(this.data.buffer,this.offset,b);this.offset+=b;return this.AbToStr(c,b-1)};BSONParser.prototype.parseBinary=function(){var b=this.readInt32();this.offset++;var c=new Uint8Array(this.data.buffer,this.offset,b);this.offset+=b;return c};
function BSONEncoder(){this.buffer=[];this.offset=0}BSONEncoder.prototype.writeByte=function(b){this.buffer.push(b&255)};BSONEncoder.prototype.writeBytes=function(b){for(var c=b.byteLength,f=0;f<c;f++)this.writeByte(b[f])};BSONEncoder.prototype.writeInt32=function(b){this.buffer.push(b);this.buffer.push(b>>8);this.buffer.push(b>>16);this.buffer.push(b>>24)};BSONEncoder.prototype.writeKey=function(b){for(var c=b.length,f=0;f<c;f++)this.writeByte(b.charCodeAt(f));this.writeByte(0)};
BSONEncoder.prototype.writeString=function(b){var c=b.length;this.writeInt32(c+1);for(var f=0;f<c;f++)this.writeByte(b.charCodeAt(f));this.writeByte(0)};BSONEncoder.prototype.writeBinary=function(b){this.writeInt32(b.byteLength);this.writeByte(0);this.writeBytes(b)};BSONEncoder.prototype.pack=function(){var b=this.buffer.length,c=b+5,f=new Uint8Array(c),p=0;f[p++]=c>>0&255;f[p++]=c>>8&255;f[p++]=c>>16&255;f[p++]=c>>24&255;for(c=0;c<b;c++)f[p++]=this.buffer[c];f[p++]=0;return f};
BSONEncoder.prototype.encode=function(b){for(var c in b)this.encodeItem(c,b[c]);return this.pack()};
BSONEncoder.prototype.encodeItem=function(b,c){if("string"==typeof c)this.writeByte(2),this.writeKey(b),this.writeString(c);else if(c instanceof ArrayBuffer)this.writeByte(5),this.writeKey(b),this.writeBinary(c);else if(null==c||"undefined"==typeof c)this.writeByte(10),this.writeKey(b);else if("object"==typeof c)this.writeByte(3),this.writeKey(b),this.writeBytes((new BSONEncoder).encode(c));else if("number"==typeof c)this.writeByte(16),this.writeKey(b),this.writeInt32(c);else if("boolean"==typeof c)this.writeByte(8),
this.writeKey(b),!0==c?this.writeByte(1):this.writeByte(0);else throw"Unrecognized encode data type";};
function RemoteVizRenderArea(b,c,f,p){this.version=1;this.img=new Image;this.canvas="string"===typeof b?document.getElementById(b):b;if("CANVAS"!==this.canvas.nodeName)window.alert("Canvas must be of type canvas");else{this.player=new Player({useWorker:!0,workerFile:"js/Decoder.js",webgl:!0});this.player.onPictureDecoded=function(){var a=new BSONEncoder;this.websocket.send(a.encode({type:"ack"}));this.redraw()}.bind(this);this.ctx=this.canvas.getContext("2d");this.renderAreaHeight=this.renderAreaWidth=0;this.displayMode=p||
"FIT";this.canvas.width=c;this.canvas.height=f;this.imageOriY=this.imageOriX=this.imageHeight=this.imageWidth=0;var m=this.clearCanvas=!1,q=!1;this.canvas.ondragstart=function(){return!1};this.events={};var k=function(a,e,h){h.events.hasOwnProperty(a)?h.events[a].push(e):h.events[a]=[e]},l=function(a,e,h){h.events.hasOwnProperty(a)&&(e=h.events[a].indexOf(e),-1!=e&&h.events[a].splice(e,1))};this.addServiceListener=function(a){k("ServiceEvent",a,this)};this.removeServiceListener=function(a){l("ServiceEvent",
a,this)};this.addReceivedImageListener=function(a){k("ReceivedImageEvent",a,this)};this.removeReceivedImageListener=function(a){l("ReceivedImageEvent",a,this)};this.addResizeListener=function(a){k("ResizeEvent",a,this)};this.removeResizeListener=function(a){l("ResizeEvent",a,this)};this.addMessageListener=function(a){k("MessageEvent",a,this)};this.removeMessageListener=function(a){l("MessageEvent",a,this)};this.addMouseMoveListener=function(a){k("MouseMoveEvent",a,this)};this.removeMouseMoveListener=
function(a){l("MouseMoveEvent",a,this)};this.addMouseDownListener=function(a){k("MouseDownEvent",a,this)};this.removeMouseDownListener=function(a){l("MouseDownEvent",a,this)};this.addMouseUpListener=function(a){k("MouseUpEvent",a,this)};this.removeMouseUpListener=function(a){l("MouseUpEvent",a,this)};this.addMouseOverListener=function(a){k("MouseOverEvent",a,this)};this.removeMouseOverListener=function(a){l("MouseOverEvent",a,this)};this.addMouseOutListener=function(a){k("MouseOutEvent",a,this)};
this.removeMouseOutListener=function(a){l("MouseOutEvent",a,this)};this.addMouseWheelListener=function(a){k("MouseWheelEvent",a,this)};this.removeMouseWheelListener=function(a){l("MouseWheelEvent",a,this)};this.addTouchStartListener=function(a){k("TouchStartEvent",a,this)};this.removeTouchStartListener=function(a){l("TouchStartEvent",a,this)};this.addTouchEndListener=function(a){k("TouchEndEvent",a,this)};this.removeTouchEndListener=function(a){l("TouchEndEvent",a,this)};this.addTouchCancelListener=
function(a){k("TouchCancelEvent",a,this)};this.removeTouchCancelListener=function(a){l("TouchCancelEvent",a,this)};this.addTouchMoveListener=function(a){k("TouchMoveEvent",a,this)};this.removeTouchMoveListener=function(a){l("TouchMoveEvent",a,this)};this.addKeyUpListener=function(a){k("KeyUpEvent",a,this)};this.removeKeyUpListener=function(a){l("KeyUpEvent",a,this)};this.addKeyDownListener=function(a){k("KeyDownEvent",a,this)};this.removeKeyDownListener=function(a){l("KeyDownEvent",a,this)};var d=
function(a,e,h){if(!a.events.hasOwnProperty(h))return!0;a=a.events[h];h=a.length;if(0==h)return!0;for(var b=!1,c=0;c<h;c++)b|=a[c](e);return b},z=function(a,e){if(a.events.hasOwnProperty("ReceivedImageEvent")){e&&e.length||(e=[]);for(var h=a.events.ReceivedImageEvent,b=h.length,c=0;c<b;c++)h[c].apply(null,e)}},t=function(a,e){if(a.events.hasOwnProperty("ServiceEvent")){e&&e.length||(e=[]);for(var h=a.events.ServiceEvent,b=h.length,c=0;c<b;c++)h[c].apply(null,e)}};!window.WebSocket&&window.MozWebSocket?
window.WebSocket=window.MozWebSocket:window.WebSocket||window.MozWebSocket||alert("Your browser does not support WebSocket.");var g=function(a,e){return function(){return e.apply(a,arguments)}};this.sendMessage=function(a){if(m&&null!=this.websocket){var e={type:"command"};e.message=a.toString();a=new BSONEncoder;this.websocket.send(a.encode(e))}};this.requestRenderAreaSize=function(a,e){if(m&&null!=this.websocket){var h={type:"requestsize"};h.width=parseInt(a);h.height=parseInt(e);var b=new BSONEncoder;
this.websocket.send(b.encode(h))}};this.getContainerWidth=function(){return this.canvas.width};this.getContainerHeight=function(){return this.canvas.height};this.getRenderAreaWidth=function(){return this.renderAreaWidth};this.getRenderAreaHeight=function(){return this.renderAreaHeight};this.resizeRenderAreaContainer=function(a,e){this.canvas.width=a;this.canvas.height=e;this.updateImageSizeAndPosition(!0);if(m&&null!=this.websocket){var h={type:"resizecontainer"};h.width=parseInt(a);h.height=parseInt(e);
var b=new BSONEncoder;this.websocket.send(b.encode(h))}};var u=function(a,e){return Math.round((e-a.imageOriX)*a.renderAreaWidth/a.imageWidth)},v=function(a,e){return Math.round((e-a.imageOriY)*a.renderAreaHeight/a.imageHeight)},r=function(a,e,h){"mousedown"==e?q=!0:"mouseup"==e&&(q=!1);var b=a.canvas.getBoundingClientRect(),c=u(a,h.clientX-b.left),d=v(a,a.canvas.height-(h.clientY-b.top));m&&null!=a.websocket&&(b={type:"mouseevent"},b.event=e,b.x=c,b.y=d,b.button=h.button,e=new BSONEncoder,a.websocket.send(e.encode(b)));
h.preventDefault()},x=function(a,e,h){if(m&&null!=a.websocket){var b={type:"keyevent"};b.event=e;b.key=h.keyCode;e=new BSONEncoder;a.websocket.send(e.encode(b))}h.preventDefault()},s=function(a,e,b){for(var c=b.changedTouches,d=0;d<c.length;d++){var f=c[d],g=a.canvas.getBoundingClientRect(),k=u(a,f.clientX-g.left),l=v(a,a.canvas.height-(f.clientY-g.top));m&&null!=a.websocket&&(g={type:"touchevent"},g.event=e,g.id=f.identifier,g.x=k,g.y=l,f=new BSONEncoder,a.websocket.send(f.encode(g)))}b.preventDefault()},
n=function(a,e,b){"down"==e&&a.canvas.focus();if(b.pointerType==b.MSPOINTER_TYPE_MOUSE||"mouse"==b.pointerType)"up"==e?q=!1:"down"==e&&(q=!0);var c=a.canvas.getBoundingClientRect(),d=u(a,b.clientX-c.left),f=v(a,a.canvas.height-(b.clientY-c.top));if(m&&null!=a.websocket){var g=e;if(b.pointerType==b.MSPOINTER_TYPE_MOUSE||"mouse"==b.pointerType)c={type:"mouseevent"},c.event="mouse"+e,c.x=d,c.y=f,c.button=0!=b.which?b.which-1:-1,e=new BSONEncoder,a.websocket.send(e.encode(c));else if(b.pointerType==b.MSPOINTER_TYPE_TOUCH||
"touch"==b.pointerType){if("down"==e||"enter"==e)g="start";else if("up"==e||"cancel"==e||"leave"==e)g="end";c={type:"touchevent"};c.event="touch"+g;c.x=d;c.y=f;c.id=b.pointerId;e=new BSONEncoder;a.websocket.send(e.encode(c))}}b.preventDefault()},w=function(a,b){if(m&&null!=a.websocket){var c={type:"mouseevent",event:"mousewheel"};c.delta=b.wheelDelta;var d=new BSONEncoder;a.websocket.send(d.encode(c))}b.preventDefault()};b=function(a){if(m){var b={type:"mouseevent",event:"mousewheel"};b.delta=120*
-(a.detail/3);var c=new BSONEncoder;this.websocket.send(c.encode(b))}a.preventDefault()};"undefined"!=typeof this.canvas.style.msTouchAction&&(this.canvas.style.msTouchAction="none");"undefined"!=typeof this.canvas.style.TouchAction&&(this.canvas.style.TouchAction="none");this.updateImageSizeAndPosition=function(a){if("STRETCH"==this.displayMode)this.imageWidth=this.ctx.canvas.width,this.imageHeight=this.ctx.canvas.height,this.imageOriY=this.imageOriX=0;else if("CROP"==this.displayMode){var b=this.renderAreaWidth/
this.ctx.canvas.width,c=this.renderAreaHeight/this.ctx.canvas.height;b>c?(this.imageWidth=Math.round(this.renderAreaWidth/c),this.imageHeight=this.ctx.canvas.height,this.imageOriX=Math.round((this.ctx.canvas.width-this.imageWidth)/2),this.imageOriY=0):b<c?(this.imageWidth=this.ctx.canvas.width,this.imageHeight=Math.round(this.renderAreaHeight/b),this.imageOriX=0,this.imageOriY=Math.round((this.ctx.canvas.height-this.imageHeight)/2)):(this.imageWidth=this.ctx.canvas.width,this.imageHeight=this.ctx.canvas.height,
this.imageOriY=this.imageOriX=0)}else b=this.renderAreaWidth/this.ctx.canvas.width,c=this.renderAreaHeight/this.ctx.canvas.height,b>c?(this.imageWidth=this.ctx.canvas.width,this.imageHeight=Math.round(this.renderAreaHeight/b),this.imageOriX=0,this.imageOriY=Math.round((this.ctx.canvas.height-this.imageHeight)/2),this.clearCanvas=!0):b<c?(this.imageWidth=Math.round(this.renderAreaWidth/c),this.imageHeight=this.ctx.canvas.height,this.imageOriX=Math.round((this.ctx.canvas.width-this.imageWidth)/2),this.imageOriY=
0,this.clearCanvas=!0):(this.imageWidth=this.ctx.canvas.width,this.imageHeight=this.ctx.canvas.height,this.imageOriY=this.imageOriX=0);a&&this.redraw()};this.redraw=function(){this.clearCanvas&&(this.ctx.clearRect(0,0,this.ctx.canvas.width,this.ctx.canvas.height),this.clearCanvas=!1);this.ctx.drawImage(this.player.canvas,this.imageOriX,this.imageOriY,this.imageWidth,this.imageHeight)};c=function(){var a=navigator.userAgent.toLowerCase();return-1!=a.indexOf("msie")?parseInt(a.split("msie")[1]):!1};
window.navigator.msPointerEnabled||window.navigator.pointerEnabled?10==c()?(this.canvas.onmspointerdown=g(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseDownEvent")||a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchStartEvent"))&&n(this,"down",a)}),this.canvas.onmspointermove=g(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseMoveEvent")||a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchMoveEvent"))&&n(this,"move",a)}),this.canvas.onmspointerup=
g(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseUpEvent")||a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchEndEvent"))&&n(this,"up",a)}),this.canvas.onmspointercancel=g(this,function(a){a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchCancelEvent")&&n(this,"cancel",a)}),this.canvas.onmspointerout=g(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseOutEvent")||a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchEndEvent"))&&n(this,"leave",
a)}),this.canvas.onmspointerover=g(this,function(a){(a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseOverEvent")||a.pointerType==a.MSPOINTER_TYPE_TOUCH&&d(this,a,"TouchStartEvent"))&&n(this,"enter",a)}),this.canvas.onmousewheel=g(this,function(a){d(this,a,"MouseWheelEvent")&&w(this,a)}),b=g(this,function(a){q&&a.pointerType==a.MSPOINTER_TYPE_MOUSE&&d(this,a,"MouseUpEvent")&&n(this,"up",a)}),window.addEventListener("MSPointerUp",b,!1)):(this.canvas.onpointerdown=g(this,function(a){("mouse"==
a.pointerType&&d(this,a,"MouseDownEvent")||"touch"==a.pointerType&&d(this,a,"TouchStartEvent"))&&n(this,"down",a)}),this.canvas.onpointermove=g(this,function(a){("mouse"==a.pointerType&&d(this,a,"MouseMoveEvent")||"touch"==a.pointerType&&d(this,a,"TouchMoveEvent"))&&n(this,"move",a)}),this.canvas.onpointerup=g(this,function(a){("mouse"==a.pointerType&&d(this,a,"MouseUpEvent")||"touch"==a.pointerType&&d(this,a,"TouchEndEvent"))&&n(this,"up",a)}),this.canvas.onpointercancel=g(this,function(a){"touch"==
a.pointerType&&d(this,a,"TouchCancelEvent")&&n(this,"cancel",a)}),this.canvas.onpointerout=g(this,function(a){("mouse"==a.pointerType&&d(this,a,"MouseOutEvent")||"touch"==a.pointerType&&d(this,a,"TouchEndEvent"))&&n(this,"leave",a)}),this.canvas.onpointerover=g(this,function(a){("mouse"==a.pointerType&&d(this,a,"MouseOverEvent")||"touch"==a.pointerType&&d(this,a,"TouchStartEvent"))&&n(this,"enter",a)}),this.canvas.onmousewheel=g(this,function(a){d(this,a,"MouseWheelEvent")&&w(this,a)}),b=g(this,function(a){q&&
"mouse"==a.pointerType&&d(this,a,"MouseUpEvent")&&n(this,"up",a)}),window.addEventListener("pointerup",b,!1)):(this.canvas.ontouchstart=g(this,function(a){d(this,a,"TouchStartEvent")&&s(this,"touchstart",a)}),this.canvas.ontouchmove=g(this,function(a){d(this,a,"TouchMoveEvent")&&s(this,"touchmove",a)}),this.canvas.ontouchend=g(this,function(a){d(this,a,"TouchEndEvent")&&s(this,"touchend",a)}),this.canvas.ontouchcancel=g(this,function(a){d(this,a,"TouchCancelEvent")&&s(this,"touchend",a)}),this.canvas.onmousedown=
g(this,function(a){d(this,a,"MouseDownEvent")&&r(this,"mousedown",a)}),this.canvas.onmouseover=g(this,function(a){d(this,a,"MouseOverEvent")&&r(this,"mouseenter",a)}),this.canvas.onmouseout=g(this,function(a){d(this,a,"MouseOutEvent")&&r(this,"mouseleave",a)}),this.canvas.onmouseup=g(this,function(a){d(this,a,"MouseUpEvent")&&r(this,"mouseup",a)}),this.canvas.onmousemove=g(this,function(a){d(this,a,"MouseMoveEvent")&&r(this,"mousemove",a)}),this.canvas.onmousewheel=g(this,function(a){d(this,a,"MouseWheelEvent")&&
w(this,a)}),b=b.bind(this),this.canvas.addEventListener("DOMMouseScroll",b,!1),b=g(this,function(a){q&&d(this,a,"MouseUpEvent")&&r(this,"mouseup",a)}),window.addEventListener("mouseup",b,!1));this.canvas.onkeyup=g(this,function(a){d(this,a,"KeyUpEvent")&&x(this,"keyup",a)});this.canvas.onkeydown=g(this,function(a){d(this,a,"KeyDownEvent")&&x(this,"keydown",a)});this.canvas.setAttribute("tabindex","0");this.canvas.focus();var A=function(a){if(m&&null!=a.websocket){var b=new BSONEncoder;a.websocket.send(b.encode({type:"alive"}))}},
y=function(){var a=(new Date).getTime();return"xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(/[xy]/g,function(b){var c=(a+16*Math.random())%16|0;a=Math.floor(a/16);return("x"==b?c:c&7|8).toString(16)})};this.wsonopen=function(){var a;var b=this.svcUrl;a=this.canvas.width;var c=this.canvas.height,d=b.match("^((ws|wss)://)([^/]*)")||[];a=0<d.length?-1==b.indexOf("requestedHeight")&&-1==b.indexOf("requestedWidth")?-1==b.indexOf("?")?b.slice(d[0].length+1,b.length)+"?requestedWidth="+a+"&requestedHeight="+
c:b.slice(d[0].length+1,b.length)+"&requestedWidth="+a+"&requestedHeight="+c:b.slice(d[0].length+1,b.length):null;null!=a?("undefined"!=typeof localStorage?localStorage.getItem("RemoteViz")?c=localStorage.getItem("RemoteViz"):(c=y(),localStorage.setItem("RemoteViz",c)):c=y(),b={type:"connection"},b.protocolversion=this.version,b.url=a,b.id=c,b.applicationname=window.location.host,b.containerwidth=this.canvas.width,b.containerheight=this.canvas.height,a=new BSONEncoder,this.websocket.send(a.encode(b))):
alert("Bad URL format.")};this.wsonclose=function(a){if(m){m=!1;var b=this;setTimeout(function(){t(b,[["disconnected","NETWORKFAILURE"]])},1)}};this.parseH264=function(a){var b=(new DataView(a)).getUint32(0,!0),c=new Uint32Array(a,4,2*b);a=new Uint8Array(a,4*(2*b+1));for(b=0;b<c.length;b+=2)this.player.decode(a.slice(c[b],c[b+1]))};this.wsonmessage=function(a){if(null!=this.websocket){var b=this,c=(new BSONParser).parse(new Uint8Array(a.data));"accept"==c.type?(this.renderAreaWidth=c.width,this.renderAreaHeight=
c.height,this.updateImageSizeAndPosition(!1),m=!0,setInterval(A,5E3,b),setTimeout(function(){t(b,[["connected",c.width,c.height]])},1)):m&&"image"==c.type?("video/h264"===c.contenttype&&this.parseH264(c.data.buffer.slice(c.data.byteOffset,c.data.byteOffset+c.data.byteLength)),z(this,[c.data.length])):m&&"command"==c.type?setTimeout(function(){var a=[c.message];if(b.events.hasOwnProperty("MessageEvent")){a&&a.length||(a=[]);for(var d=b.events.MessageEvent,f=d.length,g=0;g<f;g++)d[g].apply(null,a)}},
1):m&&"resize"==c.type?(this.renderAreaWidth=c.width,this.renderAreaHeight=c.height,this.updateImageSizeAndPosition(!1),setTimeout(function(){var a=[[c.width,c.height]];if(b.events.hasOwnProperty("ResizeEvent")){a&&a.length||(a=[]);for(var d=b.events.ResizeEvent,g=d.length,f=0;f<g;f++)d[f].apply(null,a)}},1)):"disconnect"==c.type&&setTimeout(function(){t(b,[["disconnected",c.reason]])},1)}};this.connectTo=function(a){this.svcUrl=a;this.websocket=new WebSocket(a);this.websocket.binaryType="arraybuffer";
this.websocket.onopen=g(this,this.wsonopen);this.websocket.onclose=g(this,this.wsonclose);this.websocket.onmessage=g(this,this.wsonmessage)};this.disconnect=function(){null!=this.websocket&&(this.websocket.close(),this.websocket=null,m=!1)};this.isConnected=function(){return null!=this.websocket?1==this.websocket.readyState:!1}}};