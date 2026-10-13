const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const tm = require('vscode-textmate');
const onig = require('vscode-oniguruma');
const yaml = require('js-yaml');
const root = path.resolve(__dirname, '../..');
(async () => {
  const wasm = fs.readFileSync(require.resolve('vscode-oniguruma/release/onig.wasm'));
  await onig.loadWASM(wasm.buffer.slice(wasm.byteOffset, wasm.byteOffset + wasm.byteLength));
  const grammar = JSON.parse(fs.readFileSync(path.join(root, 'editors/vscode/syntaxes/mdprompt.tmLanguage.json'), 'utf8'));
  // Minimal host grammar checks nested injections without requiring a VS Code install.
  const fallbackHost = {scopeName:'text.html.markdown',patterns:[{name:'markup.heading.markdown',begin:'^#{1,6} ',end:'$',patterns:[]}]};
  const host = process.env.MDPROMPT_MARKDOWN_GRAMMAR ? JSON.parse(fs.readFileSync(process.env.MDPROMPT_MARKDOWN_GRAMMAR, 'utf8')) : fallbackHost;
  const registry = new tm.Registry({
    onigLib: Promise.resolve({createOnigScanner:s => new onig.OnigScanner(s), createOnigString:s => new onig.OnigString(s)}),
    loadGrammar: async scope => scope === grammar.scopeName ? grammar : scope === host.scopeName ? host : null
  });
  const loaded = await registry.loadGrammar(grammar.scopeName);
  function tokenize(source) {
    let state = tm.INITIAL;
    return source.split('\n').map(line => {
      const result = loaded.tokenizeLine(line, state); state = result.ruleStack;
      return result.tokens.map(t => ({text:line.slice(t.startIndex,t.endIndex),scopes:t.scopes}));
    });
  }
  const instructions = source => tokenize(source).flat().filter(t => t.scopes.some(s => s.startsWith('meta.instruction')));
  for (const source of ['a ^%edit%^ b', '^%%^', '# title ^%edit%^', '^%%\nedit\n%%^', String.raw`\\^%edit%^`]) {
    assert.ok(instructions(source).length, source);
  }
  for (const source of ['`^%code%^`', '```\n^%code%^\n````', '~~~\n^%code%^\n~~~~', '$^%math%^$', '$$^%math%^$$', '$$\n^%%\nmath\n%%^\n$$', String.raw`\^%literal%^`]) {
    assert.equal(instructions(source).length, 0, source);
  }
  const screenshot = ['```mdprompt', '가나다^%코드 안에서는 지시가 아니다.%^ 줄글텍스트', '이것은 일반 줄글텍스트가 의도되어야 함', '```', '밖 ^%실제 지시%^'];
  const screenshotTokens = tokenize(screenshot.join('\n'));
  for (const index of [1, 2]) {
    assert.equal(screenshotTokens[index].length, 1, 'code body must remain one plain token');
    assert.ok(screenshotTokens[index][0].scopes.includes('markup.raw.block.mdprompt'));
    assert.ok(!screenshotTokens[index][0].scopes.some(s => s.startsWith('meta.instruction')));
  }
  assert.ok(screenshotTokens[4].some(t => t.scopes.includes('meta.instruction.inline.mdprompt')));
  const sublime = yaml.load(fs.readFileSync(path.join(root, 'editors/sublime/mdprompt.sublime-syntax'), 'utf8'));
  let regexCount = 0;
  function compile(value) {
    if (!value || typeof value !== 'object') return;
    for (const [key, child] of Object.entries(value)) {
      if (['match','begin','end'].includes(key) && typeof child === 'string') {
        // Dynamic fence/code backreferences refer to the parent context.
        new onig.OnigScanner([child.replace(/\\1/g, '```')]); regexCount++;
      } else compile(child);
    }
  }
  compile(sublime);
  assert.ok(regexCount > 10);
  assert.equal(fs.readFileSync(path.join(root,'editors/textmate/mdprompt.tmLanguage.json'),'utf8'), fs.readFileSync(path.join(root,'editors/vscode/syntaxes/mdprompt.tmLanguage.json'),'utf8'));
  console.log('TextMate tokenization and Sublime regex validation passed.');
})().catch(error => { console.error(error); process.exitCode = 1; });
