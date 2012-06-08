
var LibrarySinkJs = {
  $SinkJs: {
    writeFunc: null,
    frames: null,
    sizeOfFrame: null,
    channels: null,
    nativeBufferSize: null,
    nativeBuffer: null,
    nativeBufferToJavascriptBuffer: null,
    sink: null,
    proxy: null
  },

  sinkJsInit__deps: ['$SinkJs'],
  sinkJsInit: function(writeFunc, frames, sizeOfFrame, channels) {
    SinkJs.writeFunc = writeFunc;
    SinkJs.frames = frames;
    SinkJs.sizeOfFrame = sizeOfFrame;
    SinkJs.channels = channels;
    SinkJs.nativeBufferSize = (SinkJs.frames * SinkJs.sizeOfFrame);
    SinkJs.nativeBuffer = _malloc(SinkJs.nativeBufferSize);
    SinkJs.nativeBufferToJavascriptBuffer = function(nativeBuffer, javascriptBuffer, sizeOfFrame) {
      for (var i=0; i<javascriptBuffer.length; i++) {
        javascriptBuffer[i] = ({{{ makeGetValue('nativeBuffer', 'i*sizeOfFrame', 'i16', 0, 0) }}}) / 0x8000;
      }
    };
    SinkJs.sink = Sink();
    SinkJs.proxy = SinkJs.sink.createProxy(SinkJs.frames, SinkJs.channels);
    SinkJs.proxy.on('audioprocess', function(buffer, channels) {
      FUNCTION_TABLE[SinkJs.writeFunc](SinkJs.nativeBuffer, SinkJs.nativeBufferSize, channels);
      SinkJs.nativeBufferToJavascriptBuffer(SinkJs.nativeBuffer, buffer, SinkJs.sizeOfFrame);
    });
  }
};

autoAddDeps(LibrarySinkJs, '$SinkJs');
mergeInto(LibraryManager.library, LibrarySinkJs);
