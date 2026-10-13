// Visual ranges only; the native parser determines AI instruction boundaries.
function plainCodeBlocks(lines) {
  const blocks = [];
  let fence = null;
  let raw = null;
  for (let line = 0; line < lines.length; line++) {
    const text = lines[line];
    if (fence) {
      const closing = /^ {0,3}(`+|~+)[ \t]*$/.exec(text);
      if (closing && closing[1][0] === fence.character && closing[1].length >= fence.length) {
        blocks.push({ start: fence.start, end: line });
        fence = null;
      }
      continue;
    }
    if (raw) {
      if (raw === 'instruction' ? /^ {0,3}%%\^[ \t]*$/.test(text) : /^ {0,3}\$\$[ \t]*$/.test(text)) raw = null;
      continue;
    }
    if (/^ {0,3}\^%%[ \t]*$/.test(text)) { raw = 'instruction'; continue; }
    if (/^ {0,3}\$\$[ \t]*$/.test(text)) { raw = 'math'; continue; }
    const opening = /^ {0,3}(`{3,})([^`]*)$/.exec(text) || /^ {0,3}(~{3,})(.*)$/.exec(text);
    if (opening) fence = { character: opening[1][0], length: opening[1].length, start: line + 1 };
  }
  if (fence) blocks.push({ start: fence.start, end: lines.length });
  return blocks;
}
module.exports = { plainCodeBlocks };
