"use strict";
var visualizer;
(function (visualizer) {
    var K_FIX = 8;
    var offsetxs = [0, 1, 2, 0, 2, 0, 1, 2];
    var offsetys = [0, 0, 0, 1, 1, 2, 2, 2];
    var COLOR_GUIDE = [
        "rgba(255, 0, 0, 0.4)",
        "rgba(0, 0, 255, 0.4)"
    ];
    var COLOR_DICT = {
        ".": "white",
        "#": "chocolate",
        "x": "midnightblue",
        "o": "yellow"
    };
    var current_grids;
    var current_timer_id = null;
    var getInt = function (s, lineno) {
        if (s == null) {
            throw new Error("\u6570\u5024\u306E\u30D1\u30FC\u30B9\u306B\u5931\u6557\u3057\u307E\u3057\u305F (" + lineno + "\u884C\u76EE)");
        }
        if (s.match(/^\d+$/)) {
            return parseInt(s);
        }
        throw new Error("\u6570\u5024\u306E\u30D1\u30FC\u30B9\u306B\u5931\u6557\u3057\u307E\u3057\u305F (" + lineno + "\u884C\u76EE)");
    };
    var load_file = function (file, callback) {
        if (file == null)
            throw new Error("\u30D5\u30A1\u30A4\u30EB\u304C\u6307\u5B9A\u3055\u308C\u3066\u3044\u307E\u305B\u3093");
        var reader = new FileReader();
        reader.readAsText(file);
        reader.onloadend = function () { return callback(reader.result); };
    };
    var Grid = (function () {
        function Grid(a, id, conf) {
            if (a instanceof Grid) {
                var grid = a;
                this.id = grid.id;
                this.board = new Array();
                for (var i = 0; i < grid.board.length; i++) {
                    var row = new Array();
                    Object.assign(row, grid.board[i]);
                    this.board[i] = row;
                }
                this.sr = grid.sr;
                this.sc = grid.sc;
                this.trapped = grid.trapped;
                this.gain = grid.gain;
            }
            else {
                if (id == null || conf == null)
                    throw new Error();
                var lines = a;
                this.id = id;
                this.trapped = false;
                this.gain = 0;
                this.board = new Array();
                for (var i = 0; i < lines.length; i++) {
                    this.board[i] = lines[i].split("");
                }
                for (var _i = 0, _a = [0, conf.H - 1]; _i < _a.length; _i++) {
                    var r = _a[_i];
                    for (var c = 0; c < conf.W; c++) {
                        if (this.board[r][c] != "#")
                            throw new Error(id + "\u756A\u76EE\u306E\u30B0\u30EA\u30C3\u30C9\u304C\u58C1\u3067\u8986\u308F\u308C\u3066\u3044\u307E\u305B\u3093");
                    }
                }
                for (var _b = 0, _c = [0, conf.W - 1]; _b < _c.length; _b++) {
                    var c = _c[_b];
                    for (var r = 0; r < conf.H; r++) {
                        if (this.board[r][c] != "#")
                            throw new Error(id + "\u756A\u76EE\u306E\u30B0\u30EA\u30C3\u30C9\u304C\u58C1\u3067\u8986\u308F\u308C\u3066\u3044\u307E\u305B\u3093");
                    }
                }
                this.sr = this.sc = -1;
                for (var r = 1; r < conf.H - 1; r++) {
                    for (var c = 1; c < conf.W - 1; c++) {
                        if (this.board[r][c] == "@") {
                            if (this.sr == -1) {
                                this.sr = r;
                                this.sc = c;
                            }
                            else {
                                throw new Error(id + "\u756A\u76EE\u306E\u30B0\u30EA\u30C3\u30C9\u306E\u521D\u671F\u4F4D\u7F6E\u304C2\u7B87\u6240\u4EE5\u4E0A\u3042\u308A\u307E\u3059");
                            }
                        }
                    }
                }
                if (this.sr == -1) {
                    throw new Error(id + "\u756A\u76EE\u306E\u30B0\u30EA\u30C3\u30C9\u306E\u521D\u671F\u4F4D\u7F6E\u304C\u3042\u308A\u307E\u305B\u3093");
                }
                this.board[this.sr][this.sc] = ".";
            }
        }
        Grid.prototype.move = function (r, c) {
            if (this.trapped)
                return;
            var nr = this.sr + r;
            var nc = this.sc + c;
            if (this.board[nr][nc] == "#")
                return;
            if (this.board[nr][nc] == "x") {
                this.trapped = true;
                return;
            }
            if (this.board[nr][nc] == "o")
                this.gain++;
            this.board[nr][nc] = ".";
            this.sr = nr;
            this.sc = nc;
        };
        return Grid;
    }());
    var Configuration = (function () {
        function Configuration(input) {
            var lines = input.split(/\r?\n/);
            var firstLine = lines.shift();
            if (firstLine == null)
                throw new Error("\u5165\u529B\u30D5\u30A1\u30A4\u30EB\u304C\u7A7A\u3067\u3059");
            var nkhwt = firstLine.split(" ");
            if (nkhwt.length != 5)
                throw new Error("\u5165\u529B\u30D5\u30A1\u30A4\u30EB\u306E1\u884C\u76EE\u306F5\u500B\u3067\u3042\u308B\u5FC5\u8981\u304C\u3042\u308A\u307E\u3059");
            _a = nkhwt.map(function (v) { return getInt(v, 1); }), this.N = _a[0], this.K = _a[1], this.H = _a[2], this.W = _a[3], this.T = _a[4];
            if (!(this.N == 100))
                throw new Error("N\u306F100\u3067\u306A\u3051\u308C\u3070\u306A\u308A\u307E\u305B\u3093");
            if (!(this.K == K_FIX))
                throw new Error("K\u306F" + K_FIX + "\u3067\u306A\u3051\u308C\u3070\u306A\u308A\u307E\u305B\u3093");
            if (!(this.H == 50))
                throw new Error("H\u306F50\u3067\u306A\u3051\u308C\u3070\u306A\u308A\u307E\u305B\u3093");
            if (!(this.W == 50))
                throw new Error("W\u306F50\u3067\u306A\u3051\u308C\u3070\u306A\u308A\u307E\u305B\u3093");
            if (!(this.T == 2500))
                throw new Error("T\u306F2500\u3067\u306A\u3051\u308C\u3070\u306A\u308A\u307E\u305B\u3093");
            if (!(lines.length == this.N * this.H))
                throw new Error("\u5165\u529B\u30D5\u30A1\u30A4\u30EB\u306E\u884C\u6570\u306B\u904E\u4E0D\u8DB3\u304C\u3042\u308A\u307E\u3059");
            this.GRIDS = new Array();
            var num_lines = 2;
            var regexp = new RegExp("^[@ox#]{" + this.W + "}$");
            for (var i = 0; i < this.N; i++) {
                var grid = new Array();
                for (var j = 0; j < this.H; j++) {
                    var line = lines.shift();
                    if (line == null || !regexp.test(line))
                        throw new Error(num_lines + "\u884C\u76EE\u304C\u4E0D\u6B63\u3067\u3059");
                    grid[j] = line;
                    num_lines++;
                }
                this.GRIDS[i] = new Grid(grid, i, this);
            }
            var _a;
        }
        return Configuration;
    }());
    var Output = (function () {
        function Output(output, conf) {
            var lines = output.split(/\r?\n/);
            var firstLine = lines.shift();
            if (firstLine == null)
                throw new Error("\u51FA\u529B\u30D5\u30A1\u30A4\u30EB\u304C\u7A7A\u3067\u3059");
            var gs = firstLine.split(" ");
            if (gs.length != conf.K)
                throw new Error("\u51FA\u529B\u30D5\u30A1\u30A4\u30EB\u306E1\u884C\u76EE\u306F" + conf.K + "\u500B\u3067\u3042\u308B\u5FC5\u8981\u304C\u3042\u308A\u307E\u3059");
            this.GIDS = gs.map(function (v) { return getInt(v, 1); });
            for (var i = 0; i < conf.K; i++) {
                if (!(0 <= this.GIDS[i] && this.GIDS[i] < conf.N))
                    throw new Error("\u51FA\u529B\u30D5\u30A1\u30A4\u30EB\u306B\u4E0D\u6B63\u306A\u30B0\u30EA\u30C3\u30C9ID\u304C\u3042\u308A\u307E\u3059(" + this.GIDS[i] + ")");
            }
            if (new Set(this.GIDS).size != conf.K)
                throw new Error("\u51FA\u529B\u30D5\u30A1\u30A4\u30EB\u306E\u30B0\u30EA\u30C3\u30C9ID\u304C\u91CD\u8907\u3057\u3066\u3044\u307E\u3059");
            if (!(lines.length <= 1))
                throw new Error("\u51FA\u529B\u30D5\u30A1\u30A4\u30EB\u306B\u4F59\u8A08\u306A\u884C\u304C\u3042\u308A\u307E\u3059");
            var secondLine = lines.shift() || "";
            if (secondLine.length > conf.T)
                throw new Error("\u51FA\u529B\u30D5\u30A1\u30A4\u30EB\u306E2\u884C\u76EE\u306F" + conf.T + "\u6587\u5B57\u4EE5\u4E0B\u3067\u306A\u3051\u308C\u3070\u3044\u3051\u307E\u305B\u3093");
            if (!new RegExp("^[URLD]{0," + conf.T + "}$").test(secondLine))
                throw new Error("\u51FA\u529B\u30D5\u30A1\u30A4\u30EB\u306E2\u884C\u76EE\u306B\u4E0D\u6B63\u306A\u6587\u5B57\u304C\u542B\u307E\u308C\u3066\u3044\u307E\u3059\u3002");
            this.COMMANDS = secondLine;
        }
        return Output;
    }());
    visualizer.init = function () {
        var fileOfInput = document.getElementById("file-input");
        var fileOfOutput = document.getElementById("file-output");
        var scoreInput = document.getElementById("score");
        var seek = document.getElementById("seek");
        var pos = document.getElementById("pos");
        var firstButton = document.getElementById("firstButton");
        var prevButton = document.getElementById("prevButton");
        var playButton = document.getElementById("playButton");
        var nextButton = document.getElementById("nextButton");
        var lastButton = document.getElementById("lastButton");
        var maxfps = document.getElementById("maxfps");
        var exportButton = document.getElementById("exportButton");
        var canvas = document.getElementById("canvas");
        var ctx = canvas.getContext('2d');
        if (ctx == null) {
            return function () { return alert('未対応ブラウザです'); };
        }
        if (!(typeof Symbol === "function" && typeof Symbol() === "symbol")) {
            return function () { return alert('ES6未対応ブラウザです'); };
        }
        canvas.width = 800;
        canvas.height = 800;
        var M = canvas.width * 250 / 800;
        var S = canvas.width * 50 / 800 / 4;
        var simulate = function (time, conf, output) {
            var grids = [];
            for (var i = 0; i < K_FIX; i++) {
                grids[i] = new Grid(conf.GRIDS[output.GIDS[i]]);
                for (var t = 0; t < time; t++) {
                    var command = output.COMMANDS[t];
                    if (command == "U") {
                        grids[i].move(-1, 0);
                    }
                    else if (command == "D") {
                        grids[i].move(1, 0);
                    }
                    else if (command == "L") {
                        grids[i].move(0, -1);
                    }
                    else if (command == "R") {
                        grids[i].move(0, 1);
                    }
                }
            }
            return grids;
        };
        var simulateDelta = function (time, output) {
            var t = time - 1 | 0;
            for (var i = 0; i < K_FIX; i++) {
                var command = output.COMMANDS[t];
                if (command == "U") {
                    current_grids[i].move(-1, 0);
                }
                else if (command == "D") {
                    current_grids[i].move(1, 0);
                }
                else if (command == "L") {
                    current_grids[i].move(0, -1);
                }
                else if (command == "R") {
                    current_grids[i].move(0, 1);
                }
            }
        };
        var fillRoundRect = function (x, y, width, height, radius) {
            ctx.beginPath();
            ctx.moveTo(x + radius, y);
            ctx.lineTo(x + width - radius, y);
            ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
            ctx.lineTo(x + width, y + height - radius);
            ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
            ctx.lineTo(x + radius, y + height);
            ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
            ctx.lineTo(x, y + radius);
            ctx.quadraticCurveTo(x, y, x + radius, y);
            ctx.closePath();
            ctx.fill();
        };
        var drawKeyPad = function (time, output) {
            var redr = -2;
            var redc = -2;
            if (time > 0) {
                var command = output.COMMANDS.charAt(time - 1);
                if (command == "U") {
                    redr = -1;
                    redc = 0;
                }
                if (command == "D") {
                    redr = 1;
                    redc = 0;
                }
                if (command == "L") {
                    redr = 0;
                    redc = -1;
                }
                if (command == "R") {
                    redr = 0;
                    redc = 1;
                }
            }
            for (var i = -1; i <= 1; i++) {
                for (var j = -1; j <= 1; j++) {
                    if (Math.abs(i) + Math.abs(j) == 1) {
                        ctx.fillStyle = i == redc && j == redr ? "red" : "white";
                        var basex = M + 2 * S + (M / 3) + M / 6 + i * (M / 10);
                        var basey = M + 2 * S + (M / 3) + M / 6 + j * (M / 10);
                        fillRoundRect(basex - M / 10 / 2 + 1 | 0, basey - M / 10 / 2 + 1 | 0, M / 10 - 1 - 1 | 0, M / 10 - 1 - 1 | 0, 5);
                    }
                }
            }
        };
        var draw = function (time, conf, output) {
            var grids = simulate(time, conf, output);
            current_grids = grids;
            ctx.fillStyle = "black";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            var height = 22;
            ctx.font = height + "px Impact";
            ctx.textAlign = "center";
            var score = 0;
            for (var i = 0; i < K_FIX; i++) {
                var g = grids[i];
                var basex = M + 2 * S + offsetxs[i] * (M / 3) + M / 6;
                var basey = M + 2 * S + offsetys[i] * (M / 3) + M / 6 - height * 3 / 2;
                ctx.fillStyle = "aliceblue";
                ctx.fillText("ID: " + output.GIDS[i], basex | 0, basey + height | 0, M / 3 | 0);
                ctx.fillText("Score:", basex | 0, basey + 2 * height | 0, M / 3 | 0);
                ctx.fillText("" + g.gain, basex | 0, basey + 3 * height | 0, M / 3 | 0);
                score += g.gain;
            }
            scoreInput.value = "" + score;
            drawKeyPad(time, output);
            for (var i = 0; i < K_FIX; i++) {
                var g = grids[i];
                var basex = S + offsetxs[i] * (S + M);
                var basey = S + offsetys[i] * (S + M);
                for (var r = 0; r < conf.H; r++) {
                    for (var c = 0; c < conf.W; c++) {
                        var cell = g.board[r][c];
                        if (r == g.sr && c == g.sc) {
                            ctx.fillStyle = "red";
                        }
                        else {
                            ctx.fillStyle = COLOR_DICT[cell];
                        }
                        ctx.fillRect(basex + M / conf.W * c | 0, basey + M / conf.H * r | 0, M / conf.W | 0, M / conf.H | 0);
                    }
                }
                ctx.fillStyle = COLOR_GUIDE[Number(g.trapped)];
                ctx.fillRect(basex + M / conf.W * (g.sc - 2) | 0, basey + M / conf.H * (g.sr - 2) | 0, M / conf.W * 5 | 0, M / conf.H * 5 | 0);
            }
        };
        var drawDelta = function (time, conf, output) {
            simulateDelta(time, output);
            var grids = current_grids;
            ctx.fillStyle = "black";
            ctx.fillRect(2 * S + M, 2 * S + M, M, M);
            var height = 22;
            ctx.font = height + "px Impact";
            ctx.textAlign = "center";
            var score = 0;
            for (var i = 0; i < K_FIX; i++) {
                var g = grids[i];
                var basex = M + 2 * S + offsetxs[i] * (M / 3) + M / 6;
                var basey = M + 2 * S + offsetys[i] * (M / 3) + M / 6 - height * 3 / 2;
                ctx.fillStyle = "aliceblue";
                ctx.fillText("ID: " + output.GIDS[i], basex | 0, basey + height | 0, M / 3 | 0);
                ctx.fillText("Score:", basex | 0, basey + 2 * height | 0, M / 3 | 0);
                ctx.fillText("" + g.gain, basex | 0, basey + 3 * height | 0, M / 3 | 0);
                score += g.gain;
            }
            scoreInput.value = "" + score;
            drawKeyPad(time, output);
            for (var i = 0; i < K_FIX; i++) {
                var g = grids[i];
                var basex = S + offsetxs[i] * (S + M);
                var basey = S + offsetys[i] * (S + M);
                ctx.fillStyle = "black";
                ctx.fillRect(basex + (g.sc - 3) * (M / conf.W) | 0, basey + (g.sr - 3) * (M / conf.W) | 0, 7 * (M / conf.W) | 0, 7 * (M / conf.H) | 0);
                for (var r = Math.max(0, g.sr - 3); r < Math.min(conf.H, g.sr + 3 + 1); r++) {
                    for (var c = Math.max(0, g.sc - 3); c < Math.min(conf.W, g.sc + 3 + 1); c++) {
                        var cell = g.board[r][c];
                        if (r == g.sr && c == g.sc) {
                            ctx.fillStyle = "red";
                        }
                        else {
                            ctx.fillStyle = COLOR_DICT[cell];
                        }
                        ctx.fillRect(basex + M / conf.W * c | 0, basey + M / conf.H * r | 0, M / conf.W | 0, M / conf.H | 0);
                    }
                }
                ctx.fillStyle = COLOR_GUIDE[Number(g.trapped)];
                ctx.fillRect(basex + M / conf.W * (g.sc - 2) | 0, basey + M / conf.H * (g.sr - 2) | 0, M / conf.W * 5 | 0, M / conf.H * 5 | 0);
            }
        };
        var updateAndDraw = function (to, conf, output) {
            if (current_timer_id) {
                clearInterval(current_timer_id);
                current_timer_id = null;
            }
            seek.value = "" + to;
            pos.value = "" + Math.min(parseInt(seek.max), Math.max(parseInt(seek.min), to | 0));
            draw(parseInt(seek.value), conf, output);
        };
        var updateAndDrawDelta = function (conf, output) {
            if (seek.value != seek.max) {
                pos.value = seek.value = "" + (parseInt(seek.value) + 1);
                drawDelta(parseInt(seek.value), conf, output);
            }
        };
        var run = function (valueInput, valueOutput) {
            var conf = new Configuration(valueInput);
            var output = new Output(valueOutput, conf);
            seek.min = "0";
            seek.max = "" + output.COMMANDS.length;
            seek.step = "1";
            updateAndDraw(parseInt(seek.max), conf, output);
            nextButton.onclick = function (event) { return updateAndDraw(parseInt(seek.value) + 1, conf, output); };
            prevButton.onclick = function (event) { return updateAndDraw(parseInt(seek.value) - 1, conf, output); };
            lastButton.onclick = function (event) { return updateAndDraw(parseInt(seek.max), conf, output); };
            firstButton.onclick = function (event) { return updateAndDraw(parseInt(seek.min), conf, output); };
            seek.oninput = function (event) { return updateAndDraw(parseInt(seek.value), conf, output); };
            seek.onchange = function (event) { return updateAndDraw(parseInt(seek.value), conf, output); };
            pos.onchange = function (event) { return updateAndDraw(parseInt(pos.value), conf, output); };
            window.onkeydown = function (event) {
                if (event.target == seek)
                    return;
                switch (event.keyCode) {
                    case 37:
                        prevButton.click();
                        break;
                    case 39:
                        nextButton.click();
                        break;
                }
            };
            var play = function () {
                updateAndDrawDelta(conf, output);
                if (seek.value == seek.max && current_timer_id != null) {
                    clearInterval(current_timer_id);
                    current_timer_id = null;
                }
            };
            maxfps.onchange = function (event) {
                if (current_timer_id) {
                    clearInterval(current_timer_id);
                    current_timer_id = setInterval(play, 1000 / Math.max(1, parseFloat(maxfps.value)));
                }
            };
            playButton.onclick = function (event) {
                if (current_timer_id) {
                    clearInterval(current_timer_id);
                    current_timer_id = null;
                }
                else {
                    if (seek.value == seek.max)
                        updateAndDraw(parseInt(seek.min), conf, output);
                    current_timer_id = setInterval(play, 1000 / Math.max(1, parseFloat(maxfps.value)));
                }
            };
            exportButton.onclick = function () {
                var dataURL = canvas.toDataURL('image/png');
                var anchor = document.createElement('a');
                anchor.href = dataURL;
                anchor.download = 'visualizer.png';
                var e = document.createEvent('MouseEvent');
                e.initEvent("click", true, true);
                anchor.dispatchEvent(e);
            };
            playButton.focus();
        };
        return function () {
            if (current_timer_id) {
                clearInterval(current_timer_id);
                current_timer_id = null;
            }
            fileOfInput.files && load_file(fileOfInput.files[0], function (valueInput) {
                fileOfOutput.files && load_file(fileOfOutput.files[0], function (valueOutput) {
                    run(valueInput.trim(), valueOutput.trim());
                });
            });
        };
    };
})(visualizer || (visualizer = {}));
window.onload = function () {
    document.getElementById("run").onclick = visualizer.init();
};
//# sourceMappingURL=visualizer.js.map