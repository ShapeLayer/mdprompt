const vscode = require('vscode');
const { spawn } = require('node:child_process');
const { plainCodeBlocks } = require('./plain-code');
exports.activate = context => {
  // Uniform plain text also overrides semantic/theme colors inside code bodies.
  const plainCode = vscode.window.createTextEditorDecorationType({
    color: new vscode.ThemeColor('editor.foreground'),
    fontStyle: 'normal', fontWeight: 'normal', textDecoration: 'none'
  });
  function refresh(editor) {
    if (editor.document.languageId !== 'mdprompt') { editor.setDecorations(plainCode, []); return; }
    const doc = editor.document;
    const lines = Array.from({ length: doc.lineCount }, (_, line) => doc.lineAt(line).text);
    const ranges = plainCodeBlocks(lines).filter(block => block.end > block.start).map(block =>
      new vscode.Range(block.start, 0, block.end - 1, lines[block.end - 1].length));
    editor.setDecorations(plainCode, ranges);
  }
  context.subscriptions.push(plainCode,
    vscode.window.onDidChangeVisibleTextEditors(editors => editors.forEach(refresh)),
    vscode.workspace.onDidChangeTextDocument(event => {
      vscode.window.visibleTextEditors.filter(editor => editor.document === event.document).forEach(refresh);
    }));
  vscode.window.visibleTextEditors.forEach(refresh);
  context.subscriptions.push(vscode.commands.registerCommand('mdprompt.preview', async () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor || editor.document.languageId !== 'mdprompt') return;
    const executable = process.env.MDPROMPT_NATIVE || 'mdprompt-cli';
    const child = spawn(executable, ['--html']);
    let output = '', errors = '';
    child.stdout.on('data', data => output += data);
    child.stderr.on('data', data => errors += data);
    child.on('error', err => vscode.window.showErrorMessage(`mdprompt: ${err.message}`));
    child.on('close', code => {
      if (code !== 0) { vscode.window.showErrorMessage(errors || 'mdprompt parsing failed'); return; }
      const panel = vscode.window.createWebviewPanel('mdprompt', 'mdprompt content', vscode.ViewColumn.Beside, { enableScripts: false });
      panel.webview.html = `<!doctype html><html><head><meta http-equiv="Content-Security-Policy" content="default-src 'none'; style-src 'unsafe-inline'"><style>body{padding:24px;line-height:1.6}table{border-collapse:collapse}td,th{border:1px solid;padding:6px}pre{white-space:pre-wrap}</style></head><body>${output}</body></html>`;
    });
    child.stdin.on('error', () => {});
    child.stdin.end(editor.document.getText());
  }));
};
