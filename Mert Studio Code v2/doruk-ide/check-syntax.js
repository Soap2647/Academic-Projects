const fs = require('fs');
const vm = require('vm');

const files = [
  'renderer/themes.js',
  'renderer/doruk-lang.js',
  'renderer/app.js',
  'preload.js',
  'main.js',
];

files.forEach(f => {
  try {
    const code = fs.readFileSync(f, 'utf8');
    new vm.Script(code);
    console.log('✓ ' + f);
  } catch(e) {
    console.log('✗ ' + f + ' — ' + e.message);
  }
});
