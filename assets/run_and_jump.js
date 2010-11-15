      var ctx;
      var canvasHeight;
      var canvasWidth;

      var font = "sans";
      var fontsize = 16;

      var simulation = 0.0;

      var kunioImage = new Image();
      var timer;
      var sprites;
      var walk;

      var fps;

      var lastTick = (new Date()).getTime();

      var world = [
        [-5.0, 20.0, 20.0], //camera position
        [0.0, 0.0, 0.0], //camera target
        []
      ];

      function push_model(model) {
        world[2].push(model);
      }

      function make_model(file, texture, frame) {
        return [
          [file, texture, frame],
          [0.0, 0.0, 0.0],
          [0.0, 0.0, 0.0],
          [1.0, 1.0, 1.0]
        ];
      }

      function set_model_position(i, x, y, z) {
        world[2][i][1][0] = x;
        world[2][i][1][1] = y;
        world[2][i][1][2] = z;
        //world[2][0][2][0] += 10.0;
      }

      function set_model_rotation(i, x, y, z) {
        world[2][i][2][0] = x;
        world[2][i][2][1] = y;
        world[2][i][2][2] = z;
      }

      function set_model_scale(i, x, y, z) {
        world[2][i][3][0] = x;
        world[2][i][3][1] = y;
        world[2][i][3][2] = z;
      }

      function set_camera_position(x, y, z) {
        world[0][0] = x;
        world[0][1] = y;
        world[0][2] = z;
      }

      function touchstartCanvas(e) {
        X = e.touches[0].clientX;
        Y = e.touches[0].clientY;
        e.preventDefault();     
      };

      function touchmoveCanvas(e) {
        //X = e.touches[0].clientX;
        //Y = e.touches[0].clientY;
        X = e.touches[0].pageX;
        Y = e.touches[0].pageY;
        e.preventDefault(); 
      };

      function init(event) {
        if (ctx == null) {
          // Get the HTML canvas element
          var canvas = document.getElementById('canvas');

          if (canvas && canvas.getContext) {
            // save the height and width for later use by javascript
            canvasWidth = canvas.width = window.innerWidth;
            canvasHeight = canvas.height = window.innerHeight;

            // Get the canvas context, used to do all the drawing
            ctx = canvas.getContext('2d');

            // this adds the text functions to the ctx
            CanvasTextFunctions.enable(ctx);

            //var y = ctx.fontAscent(font,fontsize);
            //ctx.strokeStyle = "rgba(255, 0, 0, 1)";
            //ctx.drawTextCenter( font, fontsize, canvas.width/2, y, "Canvas Text Functions");

            //we need a skybox
            push_model(make_model(0, 2, 0));
            set_model_scale(0, 100.0, 100.0, 100.0);

            //some blocks
            for (i=0; i<10; i++) {
              push_model(make_model(0, 1, 0));
              set_model_position(i, i, 0, 0);
            }

            //timer = new FrameTimer();
            //timer.tick();

            /*
            setInterval(function() {
              ideal_dt = (1.0 / 30.0) * 1000.0;
              dt = timer.getMillisecondsSinceTick();
              ticked_dt = 0.0;
              sub_divided = 0;
              while (ticked_dt < dt) {
                ticked_dt += ideal_dt;
                simulation += ideal_dt;
                set_camera_position(world[0][0] + (0.01 * ideal_dt), world[0][1], world[0][2]);
                window.location = "#" + JSON.stringify(world);
                sub_divided++;
              }
              ctx.clearRect(0, 0, canvasWidth, canvasHeight);
              ctx.drawTextCenter(font, fontsize, canvasWidth / 2, canvasHeight / 2, "dt: " + dt + " sd:" + sub_divided);
              timer.tick();
            }, 15);
            */

            /*
            setInterval(function() {
              ctx.clearRect(0, 0, canvasWidth, canvasHeight);
              ctx.strokeStyle = "rgba(255, 0, 0, 1)";
              ctx.drawTextCenter(font, fontsize, canvasWidth / 2, canvasHeight / 2, "simulation: " + simulation);
            }, 16);
            */

            fps = document.getElementById("fps");

            setInterval(function() {
              fps.innerText = "fps: " + simulation;
            }, 1);

            setInterval(function() {
              currentTick = (new Date()).getTime();
              dt = currentTick - lastTick;
              ideal_dt = 16;
              lastTick = (new Date()).getTime();
              
              for (i=0; i<dt; i+=ideal_dt) {
                simulation += ideal_dt;
                set_camera_position(world[0][0] + (0.01 * ideal_dt), world[0][1], world[0][2]);
                //ctx.strokeStyle = "rgba(255, 0, 0, 1)";
                //ctx.drawTextCenter(font, fontsize, canvasWidth / 2, canvasHeight / 2, "simulation: " + simulation);
                //  window.location = "#" + JSON.stringify(world);
                //window.location = "#" + JSON.stringify(world);
                //ajax = new XMLHttpRequest();
                //ajax.open("GET", "foo://" + JSON.stringify(world), false);
                //ajax.send();
                window.location = "#" + JSON.stringify(world);
              }
            }, 1);

            //setInterval(function() {
            //  window.location = "#" + JSON.stringify(world);
            //}, 1);
          }
        }
      };
