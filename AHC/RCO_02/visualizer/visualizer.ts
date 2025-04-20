namespace visualizer {
  const K_FIX = 8;
  const offsetxs: number[] = [0, 1, 2, 0, 2, 0, 1, 2];
  const offsetys: number[] = [0, 0, 0, 1, 1, 2, 2, 2];
  const COLOR_GUIDE: string[] = [
    "rgba(255, 0, 0, 0.4)",
    "rgba(0, 0, 255, 0.4)"
  ];
  const COLOR_DICT:{[key: string]: string} = {
    ".": "white",
    "#": "chocolate",
    "x": "midnightblue",
    "o": "yellow"
  };
  // 差分計算用
  let current_grids: Grid[];
  let current_timer_id: number|null = null;

  const getInt = (s: string, lineno: number) => {
      if (s == null) {
          throw new Error(`数値のパースに失敗しました (${lineno}行目)`);
      }
      if (s.match(/^\d+$/)) {
          return parseInt(s);
      }
      throw new Error(`数値のパースに失敗しました (${lineno}行目)`);
  };

  const load_file = (file:File, callback: (value: string) => void) => {
    if(file == null)throw new Error(`ファイルが指定されていません`);
    const reader = new FileReader();
    reader.readAsText(file);
    reader.onloadend = () => callback(reader.result);
  };

  class Grid {
    public id: number;
    public board: string[][];
    public sr: number;
    public sc: number;
    public trapped: boolean;
    public gain: number;

    public move(r: number, c: number): void {
      if(this.trapped)return;
      const nr = this.sr + r;
      const nc = this.sc + c;
      if(this.board[nr][nc] == "#")return;
      if(this.board[nr][nc] == "x"){
        this.trapped = true;
        return;
      }
      if(this.board[nr][nc] == "o")this.gain++;
      this.board[nr][nc] = ".";
      this.sr = nr; this.sc = nc;
    }

    constructor(grid: Grid);
    constructor(lines: string[], id: number, conf: Configuration);

    constructor(a: any, id?: number, conf?: Configuration) {
      if(a instanceof Grid){
        // Gridが来た場合コピーコンストラクタ
        const grid: Grid = a;
        this.id = grid.id;
        this.board = new Array();
        for(let i = 0;i < grid.board.length;i++){
          let row:string[] = new Array();
          Object.assign(row, grid.board[i]); // deep copy
          this.board[i] = row;
        }
        this.sr = grid.sr;
        this.sc = grid.sc;
        this.trapped = grid.trapped;
        this.gain = grid.gain;
      }else{
        // string[]が来た場合バリデーションつき初期作成
        if(id == null || conf == null)throw new Error();
        const lines:string[] = a;
        this.id = id;
        this.trapped = false;
        this.gain = 0;
        this.board = new Array();
        for(let i = 0;i < lines.length;i++){
          this.board[i] = lines[i].split("");
        }

        // 壁で覆われているかのバリデーション
        for(let r of [0, conf.H-1]){
          for(let c = 0;c < conf.W;c++){
            if(this.board[r][c] != "#")throw new Error(`${id}番目のグリッドが壁で覆われていません`);
          }
        }
        for(let c of [0, conf.W-1]){
          for(let r = 0;r < conf.H;r++){
            if(this.board[r][c] != "#")throw new Error(`${id}番目のグリッドが壁で覆われていません`);
          }
        }

        // 初期位置の取得とバリデーション
        this.sr = this.sc = -1;
        for(let r = 1;r < conf.H-1;r++){
          for(let c = 1;c < conf.W-1;c++){
            if(this.board[r][c] == "@"){
              if(this.sr == -1){
                this.sr = r; this.sc = c;
              }else{
                throw new Error(`${id}番目のグリッドの初期位置が2箇所以上あります`);
              }
            }
          }
        }
        if(this.sr == -1){
          throw new Error(`${id}番目のグリッドの初期位置がありません`);
        }
        this.board[this.sr][this.sc] = ".";
      }
    }
  }

  class Configuration {
    public N: number;
    public K: number;
    public H: number;
    public W: number;
    public T: number;
    public GRIDS: Grid[];

    constructor(input: string) {
      const lines = input.split(/\r?\n/);
      const firstLine = lines.shift();
      if(firstLine == null)throw new Error(`入力ファイルが空です`);
      const nkhwt = firstLine.split(" ");
      if(nkhwt.length != 5)throw new Error(`入力ファイルの1行目は5個である必要があります`);
      [this.N, this.K, this.H, this.W, this.T] = nkhwt.map(v => getInt(v, 1));

      if(!(this.N == 100))throw new Error(`Nは100でなければなりません`);
      if(!(this.K == K_FIX))throw new Error(`Kは${K_FIX}でなければなりません`);
      if(!(this.H == 50))throw new Error(`Hは50でなければなりません`);
      if(!(this.W == 50))throw new Error(`Wは50でなければなりません`);
      if(!(this.T == 2500))throw new Error(`Tは2500でなければなりません`);
      if(!(lines.length == this.N * this.H))throw new Error(`入力ファイルの行数に過不足があります`);

      // GRIDSに格納
      this.GRIDS = new Array();
      let num_lines = 2;
      const regexp = new RegExp(`^[@ox#]{${this.W}}$`);
      for(let i = 0;i < this.N;i++){
        var grid = new Array();
        for(let j = 0;j < this.H;j++){
          const line = lines.shift();
          if(line == null || !regexp.test(line))throw new Error(`${num_lines}行目が不正です`);
          grid[j] = line
          num_lines++;
        }
        this.GRIDS[i] = new Grid(grid, i, this);
      }
    }
  }

  class Output {
    public GIDS: number[];
    public COMMANDS: string;

    constructor(output: string, conf: Configuration) {
      const lines = output.split(/\r?\n/);

      const firstLine = lines.shift();
      if(firstLine == null)throw new Error(`出力ファイルが空です`);
      const gs = firstLine.split(" ");
      if(gs.length != conf.K)throw new Error(`出力ファイルの1行目は${conf.K}個である必要があります`);
      this.GIDS = gs.map(v => getInt(v, 1));
      for(let i = 0;i < conf.K;i++){
        if(!(0 <= this.GIDS[i] && this.GIDS[i] < conf.N))throw new Error(`出力ファイルに不正なグリッドIDがあります(${this.GIDS[i]})`);
      }
      if(new Set(this.GIDS).size != conf.K)throw new Error(`出力ファイルのグリッドIDが重複しています`);

      if(!(lines.length <= 1))throw new Error(`出力ファイルに余計な行があります`);
      const secondLine = lines.shift() || "";
      if(secondLine.length > conf.T)throw new Error(`出力ファイルの2行目は${conf.T}文字以下でなければいけません`);
      if(!new RegExp(`^[URLD]{0,${conf.T}}$`).test(secondLine))throw new Error(`出力ファイルの2行目に不正な文字が含まれています。`);
      this.COMMANDS = secondLine;
    }
  }

  export const init = () => {
    const fileOfInput = <HTMLInputElement> document.getElementById("file-input");
    const fileOfOutput = <HTMLInputElement> document.getElementById("file-output");
    const scoreInput = <HTMLInputElement> document.getElementById("score");

    // controls
    const seek = <HTMLInputElement> document.getElementById("seek");
    const pos = <HTMLInputElement> document.getElementById("pos");
    const firstButton = <HTMLInputElement> document.getElementById("firstButton");
    const prevButton = <HTMLInputElement> document.getElementById("prevButton");
    const playButton = <HTMLInputElement> document.getElementById("playButton");
    const nextButton = <HTMLInputElement> document.getElementById("nextButton");
    const lastButton = <HTMLInputElement> document.getElementById("lastButton");
    const maxfps = <HTMLInputElement> document.getElementById("maxfps");
    const exportButton = <HTMLInputElement> document.getElementById("exportButton");

    const canvas = <HTMLCanvasElement> document.getElementById("canvas");
    const ctx = canvas.getContext('2d');

    if (ctx == null) {
      return () => alert('未対応ブラウザです');
    }
    if(!(typeof Symbol === "function" && typeof Symbol() === "symbol")){
      return () => alert('ES6未対応ブラウザです');
    }

    canvas.width = 800;
    canvas.height = 800;
    // 3*M+4*S = width
    const M = canvas.width * 250/800; // マップ1個の幅
    const S = canvas.width * 50/800/4; // マップの間隔

    // [0,time)でシミュレート
    const simulate = (time: Number, conf: Configuration, output: Output) => {
      let grids:Grid[] = [];
      for(let i = 0;i < K_FIX;i++){
        grids[i] = new Grid(conf.GRIDS[output.GIDS[i]]);
        for(let t = 0;t < time;t++){
          const command = output.COMMANDS[t];
          if(command == "U"){
            grids[i].move(-1, 0);
          }else if(command == "D"){
            grids[i].move(1, 0);
          }else if(command == "L"){
            grids[i].move(0, -1);
          }else if(command == "R"){
            grids[i].move(0, 1);
          }
        }
      }
      return grids;
    };

    // current_gridsに対しtime-1のみシミュレート
    const simulateDelta = (time: number, output: Output) => {
      const t: number = time-1|0;
      for(let i = 0;i < K_FIX;i++){
        const command = output.COMMANDS[t];
        if(command == "U"){
          current_grids[i].move(-1, 0);
        }else if(command == "D"){
          current_grids[i].move(1, 0);
        }else if(command == "L"){
          current_grids[i].move(0, -1);
        }else if(command == "R"){
          current_grids[i].move(0, 1);
        }
      }
    };

    const fillRoundRect = (x: number, y: number, width: number, height: number, radius: number) => {
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
    }

    const drawKeyPad = (time: number, output: Output) => {
      let redr = -2;
      let redc = -2;
      if(time > 0){
        const command = output.COMMANDS.charAt(time-1);
        if(command == "U"){ redr = -1; redc = 0; }
        if(command == "D"){ redr = 1; redc = 0; }
        if(command == "L"){ redr = 0; redc = -1; }
        if(command == "R"){ redr = 0; redc = 1; }
      }

      for(let i = -1;i <= 1;i++){
        for(let j = -1;j <= 1;j++){
          if(Math.abs(i) + Math.abs(j) == 1){
            ctx.fillStyle = i == redc && j == redr ? "red" : "white";
            const basex = M + 2*S + (M/3) + M/6 + i * (M/10); // center
            const basey = M + 2*S + (M/3) + M/6 + j * (M/10);
            // ctx.fillRect(basex+3, basey+3, M/9-3-3|0, M/9-3-3|0);
            fillRoundRect(basex-M/10/2+1|0, basey-M/10/2+1|0, M/10-1-1|0, M/10-1-1|0, 5);
          }
        }
      }
    };

    // 描画
    const draw = (time: number, conf: Configuration, output: Output) => {
      // timeまでシミュレートしたグリッドを生成
      const grids = simulate(time, conf, output);
      current_grids = grids;

      // 画面クリア
      ctx.fillStyle = "black";
      ctx.fillRect(0, 0, canvas.width, canvas.height);

      // グリッドidとスコアを表示
      const height = 22;
      ctx.font = height + "px Impact";
      ctx.textAlign = "center";
      let score = 0;
      for(let i = 0;i < K_FIX;i++){
        const g = grids[i];
        const basex = M + 2*S + offsetxs[i] * (M/3) + M/6; // center
        const basey = M + 2*S + offsetys[i] * (M/3) + M/6 - height*3/2;
        ctx.fillStyle = "aliceblue";
        ctx.fillText("ID: " + output.GIDS[i], basex|0, basey+height|0, M/3|0);
        ctx.fillText("Score:", basex|0, basey+2*height|0, M/3|0);
        ctx.fillText("" + g.gain, basex|0, basey+3*height|0, M/3|0);
        score += g.gain;
      }
      scoreInput.value = "" + score;

      // キーパッドを描画
      drawKeyPad(time, output);

      // ■でぬる
      for(let i = 0;i < K_FIX;i++){
        const g = grids[i];
        const basex = S + offsetxs[i] * (S + M);
        const basey = S + offsetys[i] * (S + M);
        for(let r = 0;r < conf.H;r++){
          for(let c = 0;c < conf.W;c++){
            const cell = g.board[r][c];
            if(r == g.sr && c == g.sc){
              ctx.fillStyle = "red";
            }else{
              ctx.fillStyle = COLOR_DICT[cell];
            }
            ctx.fillRect(
              basex + M / conf.W * c | 0,
              basey + M / conf.H * r | 0,
              M / conf.W | 0,
              M / conf.H | 0,
            )
          }
        }

        // ガイド
        ctx.fillStyle = COLOR_GUIDE[Number(g.trapped)];
        ctx.fillRect(
          basex + M / conf.W * (g.sc - 2) | 0,
          basey + M / conf.H * (g.sr - 2) | 0,
          M / conf.W * 5 | 0,
          M / conf.H * 5 | 0,
        )
      }
    };

    // 差分描画
    const drawDelta = (time: number, conf: Configuration, output: Output) => {
      // current_gridsを1時間単位進める
      simulateDelta(time, output);
      const grids = current_grids;

      // 中央部のみクリア
      ctx.fillStyle = "black";
      ctx.fillRect(2*S+M, 2*S+M, M, M);

      // グリッドidとスコアを表示
      const height = 22;
      ctx.font = height + "px Impact";
      ctx.textAlign = "center";
      let score = 0;
      for(let i = 0;i < K_FIX;i++){
        const g = grids[i];
        const basex = M + 2*S + offsetxs[i] * (M/3) + M/6; // center
        const basey = M + 2*S + offsetys[i] * (M/3) + M/6 - height*3/2;
        ctx.fillStyle = "aliceblue";
        ctx.fillText("ID: " + output.GIDS[i], basex|0, basey+height|0, M/3|0);
        ctx.fillText("Score:", basex|0, basey+2*height|0, M/3|0);
        ctx.fillText("" + g.gain, basex|0, basey+3*height|0, M/3|0);
        score += g.gain;
      }
      scoreInput.value = "" + score;

      // キーパッドを描画
      drawKeyPad(time, output);

      // ■でぬる
      for(let i = 0;i < K_FIX;i++){
        const g = grids[i];
        const basex = S + offsetxs[i] * (S + M);
        const basey = S + offsetys[i] * (S + M);

        // 7x7の領域のみクリア
        ctx.fillStyle = "black";
        ctx.fillRect(
          basex+(g.sc-3)*(M / conf.W)|0,
          basey+(g.sr-3)*(M / conf.W)|0,
          7*(M / conf.W)|0,
          7*(M / conf.H)|0
        );

        for(let r = Math.max(0, g.sr-3);r < Math.min(conf.H, g.sr+3+1);r++){
          for(let c = Math.max(0, g.sc-3);c < Math.min(conf.W, g.sc+3+1);c++){
            const cell = g.board[r][c];
            if(r == g.sr && c == g.sc){
              ctx.fillStyle = "red";
            }else{
              ctx.fillStyle = COLOR_DICT[cell];
            }
            ctx.fillRect(
              basex + M / conf.W * c | 0,
              basey + M / conf.H * r | 0,
              M / conf.W | 0,
              M / conf.H | 0,
            )
          }
        }

        // ガイド
        ctx.fillStyle = COLOR_GUIDE[Number(g.trapped)];
        ctx.fillRect(
          basex + M / conf.W * (g.sc - 2) | 0,
          basey + M / conf.H * (g.sr - 2) | 0,
          M / conf.W * 5 | 0,
          M / conf.H * 5 | 0,
        )
      }
    };

    const updateAndDraw = (to: number, conf: Configuration, output: Output) => {
      if(current_timer_id){
        clearInterval(current_timer_id);
        current_timer_id = null;
      }
      seek.value = "" + to;
      pos.value = "" + Math.min(parseInt(seek.max), Math.max(parseInt(seek.min), to|0));
      draw(parseInt(seek.value), conf, output);
    };

    const updateAndDrawDelta = (conf: Configuration, output: Output) => {
      if(seek.value != seek.max){
        pos.value = seek.value = ""+ (parseInt(seek.value)+1);
        drawDelta(parseInt(seek.value), conf, output);
      }
    };

    const run = (valueInput : string, valueOutput: string) => {
      const conf = new Configuration(valueInput);
      const output = new Output(valueOutput, conf);

      // 初期状態の描画
      seek.min = "0";
      seek.max = "" + output.COMMANDS.length;
      seek.step = "1";
      updateAndDraw(parseInt(seek.max), conf, output); // 読み込み終了時点では最終状態にする

      // イベントリスナ上書き
      nextButton.onclick = (event: MouseEvent) => updateAndDraw(parseInt(seek.value)+1, conf, output);
      prevButton.onclick = (event: MouseEvent) => updateAndDraw(parseInt(seek.value)-1, conf, output);
      lastButton.onclick = (event: MouseEvent) => updateAndDraw(parseInt(seek.max), conf, output);
      firstButton.onclick = (event: MouseEvent) => updateAndDraw(parseInt(seek.min), conf, output);
      seek.oninput = (event: Event) => updateAndDraw(parseInt(seek.value), conf, output);
      seek.onchange = (event: Event) => updateAndDraw(parseInt(seek.value), conf, output);
      pos.onchange = (event: Event) => updateAndDraw(parseInt(pos.value), conf, output);
      window.onkeydown = (event: KeyboardEvent) => {
        if (event.target == seek) return;
        switch (event.keyCode) {
          case 37:
            prevButton.click();
            break;
          case 39:
            nextButton.click();
            break;
        }
      };

      const play = () => {
        updateAndDrawDelta(conf, output);
        if(seek.value == seek.max && current_timer_id != null){
          clearInterval(current_timer_id);
          current_timer_id = null;
        }
      };
      maxfps.onchange = (event: Event) => {
        if(current_timer_id){
          clearInterval(current_timer_id);
          current_timer_id = setInterval(play, 1000/Math.max(1, parseFloat(maxfps.value)));
        }
      }

      playButton.onclick = (event: MouseEvent) => {
        if(current_timer_id){
          // 停止
          clearInterval(current_timer_id);
          current_timer_id = null;
        }else{
          // 現在位置から再生
          if(seek.value == seek.max)updateAndDraw(parseInt(seek.min), conf, output);
          current_timer_id = setInterval(play, 1000/Math.max(1, parseFloat(maxfps.value)));
        }
      };

      exportButton.onclick = () => {
        const dataURL = canvas.toDataURL('image/png');
        const anchor = document.createElement('a');
        anchor.href = dataURL;
        anchor.download = 'visualizer.png';
        const e = document.createEvent('MouseEvent');
        e.initEvent("click", true, true);
        anchor.dispatchEvent(e);

        // anchor.dispatchEvent(new CustomEvent('click'));
      };

      playButton.focus();
    };

    return () => {
      if(current_timer_id){
        // 停止
        clearInterval(current_timer_id);
        current_timer_id = null;
      }
      fileOfInput.files && load_file(fileOfInput.files[0], (valueInput: string) => {
        fileOfOutput.files && load_file(fileOfOutput.files[0], (valueOutput: string) => {
          run(valueInput.trim(), valueOutput.trim());
        });
      });
    }
  }
}

window.onload = () => {
    (<HTMLButtonElement> document.getElementById("run")).onclick = visualizer.init();
};
