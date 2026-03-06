/* ============================================================
   MERT STUDIO CODE — Multi-language syntax engine
   14 languages supported
   ============================================================ */

/* ============================================================
   LANGUAGE DEFINITIONS
   ============================================================ */
export const LANGUAGES = {
    turkce_c: {
        id: 'turkce_c', name: 'Türkçe C', extensions: ['.c.tr'],
        commentChar: '//', blockComment: ['/*', '*/'],
        keywords: ['eğer', 'değilse', 'değilse_eğer', 'döngü', 'süre', 'yap_süre', 'dön', 'tanım', 'dahil_et', 'yapı', 'sınıf', 'kamu', 'özel', 'korumalı', 'boş', 'tamsayı', 'ondalık', 'çift', 'karakter', 'metin', 'doğru', 'yanlış', 'değiştir', 'durum', 'varsayılan', 'kır', 'devam', 'sabit', 'statik'],
        functions: ['çıktı', 'değer_al', 'oku', 'yaz'],
        defaultCode: `// Türkçe C Programlama Örneği
dahil_et <standart_io>

tamsayı toplam(tamsayı a, tamsayı b) {
    dön a + b;
}

tamsayı ana() {
    tamsayı sayi1 = 10;
    tamsayı sayi2 = 20;
    tamsayı sonuc = toplam(sayi1, sayi2);
    çıktı("Toplam: %d\\n", sonuc);
    dön 0;
}`,
    },
    turkce_python: {
        id: 'turkce_python', name: 'Türkçe Python', extensions: ['.py.tr', '.tpy'],
        commentChar: '#',
        keywords: ['tanımla', 'sınıf', 'eğer', 'değilse_eğer', 'değilse', 'için', 'süre', 'dön', 'içe_aktar', 'şuradan', 'doğru', 'yanlış', 'hiçbiri', 've', 'veya', 'değil', 'dene', 'yakala', 'sonunda', 'geç', 'yükselt', 'ile', 'olarak', 'içinde', 'kır', 'devam', 'küresel', 'öz'],
        functions: ['yazdır', 'girdi', 'uzunluk', 'aralık', 'tür', 'tamsayı_çevir', 'metin_çevir', 'ondalık_çevir', 'liste', 'sözlük', 'küme', 'demet'],
        defaultCode: `# Türkçe Python Programlama Örneği

tanımla selamla(isim):
    yazdır("Merhaba, " + isim + "!")

tanımla ana():
    isim = "Dünya"
    selamla(isim)
    
    için i içinde aralık(5):
        yazdır(metin_çevir(i))

ana()
`,
    },
    javascript: {
        id: 'javascript', name: 'JavaScript', extensions: ['.js', '.jsx', '.mjs'],
        commentChar: '//', blockComment: ['/*', '*/'],
        keywords: ['const', 'let', 'var', 'function', 'return', 'if', 'else', 'for', 'while', 'do', 'switch', 'case', 'default', 'break', 'continue', 'class', 'extends', 'new', 'this', 'super', 'import', 'export', 'from', 'async', 'await', 'try', 'catch', 'finally', 'throw', 'typeof', 'instanceof', 'in', 'of', 'true', 'false', 'null', 'undefined', 'void', 'delete', 'yield', 'static', 'get', 'set', 'constructor'],
        functions: ['console.log', 'parseInt', 'parseFloat', 'setTimeout', 'setInterval', 'clearTimeout', 'clearInterval', 'fetch', 'Promise', 'Array', 'Object', 'String', 'Number', 'Math', 'JSON', 'Date', 'Map', 'Set', 'require', 'addEventListener', 'querySelector', 'getElementById'],
        defaultCode: `// JavaScript Example
function greet(name) {
    console.log(\`Hello, \${name}!\`);
}

const numbers = [1, 2, 3, 4, 5];
const sum = numbers.reduce((acc, n) => acc + n, 0);
console.log("Sum:", sum);

greet("World");
`,
    },
    typescript: {
        id: 'typescript', name: 'TypeScript', extensions: ['.ts', '.tsx'],
        commentChar: '//', blockComment: ['/*', '*/'],
        keywords: ['const', 'let', 'var', 'function', 'return', 'if', 'else', 'for', 'while', 'do', 'switch', 'case', 'default', 'break', 'continue', 'class', 'extends', 'new', 'this', 'super', 'import', 'export', 'from', 'async', 'await', 'try', 'catch', 'finally', 'throw', 'typeof', 'instanceof', 'in', 'of', 'true', 'false', 'null', 'undefined', 'void', 'delete', 'yield', 'static', 'get', 'set', 'constructor', 'interface', 'type', 'enum', 'namespace', 'module', 'declare', 'abstract', 'implements', 'readonly', 'as', 'is', 'keyof', 'infer', 'never', 'unknown', 'any', 'string', 'number', 'boolean', 'object', 'symbol', 'bigint'],
        functions: ['console.log', 'parseInt', 'parseFloat', 'setTimeout', 'setInterval', 'fetch', 'Promise', 'Array', 'Object', 'String', 'Number', 'Math', 'JSON', 'Date', 'Map', 'Set', 'require'],
        defaultCode: `// TypeScript Example
interface User {
    name: string;
    age: number;
}

function greet(user: User): string {
    return \`Hello, \${user.name}!\`;
}

const user: User = { name: "World", age: 25 };
console.log(greet(user));
`,
    },
    python: {
        id: 'python', name: 'Python', extensions: ['.py'],
        commentChar: '#',
        keywords: ['def', 'class', 'if', 'elif', 'else', 'for', 'while', 'return', 'import', 'from', 'as', 'with', 'try', 'except', 'finally', 'raise', 'pass', 'break', 'continue', 'and', 'or', 'not', 'in', 'is', 'True', 'False', 'None', 'lambda', 'yield', 'global', 'nonlocal', 'assert', 'del', 'async', 'await'],
        functions: ['print', 'input', 'len', 'range', 'type', 'int', 'str', 'float', 'list', 'dict', 'set', 'tuple', 'map', 'filter', 'zip', 'enumerate', 'sorted', 'reversed', 'min', 'max', 'sum', 'abs', 'round', 'open', 'isinstance', 'hasattr', 'getattr', 'setattr', 'super'],
        defaultCode: `# Python Example
def greet(name):
    print(f"Hello, {name}!")

numbers = [1, 2, 3, 4, 5]
total = sum(numbers)
print(f"Sum: {total}")

greet("World")
`,
    },
    java: {
        id: 'java', name: 'Java', extensions: ['.java'],
        commentChar: '//', blockComment: ['/*', '*/'],
        keywords: ['public', 'private', 'protected', 'class', 'interface', 'extends', 'implements', 'abstract', 'static', 'final', 'void', 'int', 'long', 'double', 'float', 'char', 'boolean', 'byte', 'short', 'String', 'new', 'return', 'if', 'else', 'for', 'while', 'do', 'switch', 'case', 'default', 'break', 'continue', 'try', 'catch', 'finally', 'throw', 'throws', 'import', 'package', 'this', 'super', 'null', 'true', 'false', 'instanceof', 'enum', 'synchronized', 'volatile', 'transient', 'native'],
        functions: ['System.out.println', 'System.out.print', 'Arrays.sort', 'Collections.sort', 'Integer.parseInt', 'String.valueOf', 'Math.max', 'Math.min', 'Math.random'],
        defaultCode: `// Java Example
public class Main {
    public static void main(String[] args) {
        String name = "World";
        System.out.println("Hello, " + name + "!");
        
        int[] numbers = {1, 2, 3, 4, 5};
        int sum = 0;
        for (int n : numbers) {
            sum += n;
        }
        System.out.println("Sum: " + sum);
    }
}
`,
    },
    cpp: {
        id: 'cpp', name: 'C++', extensions: ['.cpp', '.hpp', '.cc', '.hh', '.h'],
        commentChar: '//', blockComment: ['/*', '*/'],
        keywords: ['auto', 'break', 'case', 'char', 'const', 'continue', 'default', 'do', 'double', 'else', 'enum', 'extern', 'float', 'for', 'goto', 'if', 'int', 'long', 'register', 'return', 'short', 'signed', 'sizeof', 'static', 'struct', 'switch', 'typedef', 'union', 'unsigned', 'void', 'volatile', 'while', 'class', 'namespace', 'template', 'this', 'new', 'delete', 'try', 'catch', 'throw', 'public', 'private', 'protected', 'virtual', 'friend', 'inline', 'using', 'true', 'false', 'nullptr', 'bool', 'string', 'vector', 'map', 'set', 'include', 'define'],
        functions: ['cout', 'cin', 'endl', 'printf', 'scanf', 'malloc', 'free', 'sizeof', 'strlen', 'strcpy', 'push_back', 'begin', 'end', 'size', 'empty', 'find', 'insert', 'erase', 'sort', 'getline'],
        defaultCode: `// C++ Example
#include <iostream>
#include <vector>
using namespace std;

int main() {
    string name = "World";
    cout << "Hello, " << name << "!" << endl;
    
    vector<int> numbers = {1, 2, 3, 4, 5};
    int sum = 0;
    for (int n : numbers) {
        sum += n;
    }
    cout << "Sum: " << sum << endl;
    return 0;
}
`,
    },
    csharp: {
        id: 'csharp', name: 'C#', extensions: ['.cs'],
        commentChar: '//', blockComment: ['/*', '*/'],
        keywords: ['using', 'namespace', 'class', 'struct', 'interface', 'enum', 'public', 'private', 'protected', 'internal', 'static', 'void', 'int', 'long', 'double', 'float', 'char', 'bool', 'string', 'var', 'new', 'return', 'if', 'else', 'for', 'foreach', 'while', 'do', 'switch', 'case', 'default', 'break', 'continue', 'try', 'catch', 'finally', 'throw', 'this', 'base', 'null', 'true', 'false', 'abstract', 'virtual', 'override', 'sealed', 'async', 'await', 'in', 'out', 'ref', 'params', 'readonly', 'const', 'delegate', 'event', 'is', 'as', 'typeof', 'sizeof', 'nameof', 'get', 'set', 'value', 'partial', 'where', 'select', 'from', 'orderby', 'group', 'into', 'join', 'let'],
        functions: ['Console.WriteLine', 'Console.ReadLine', 'Convert.ToInt32', 'Convert.ToString', 'Math.Max', 'Math.Min', 'String.Format', 'int.Parse', 'string.Join', 'Array.Sort', 'List', 'Dictionary', 'StringBuilder'],
        defaultCode: `// C# Example
using System;
using System.Collections.Generic;

class Program {
    static void Main(string[] args) {
        string name = "World";
        Console.WriteLine($"Hello, {name}!");
        
        var numbers = new List<int> { 1, 2, 3, 4, 5 };
        int sum = 0;
        foreach (int n in numbers) {
            sum += n;
        }
        Console.WriteLine($"Sum: {sum}");
    }
}
`,
    },
    go: {
        id: 'go', name: 'Go', extensions: ['.go'],
        commentChar: '//', blockComment: ['/*', '*/'],
        keywords: ['package', 'import', 'func', 'var', 'const', 'type', 'struct', 'interface', 'map', 'chan', 'range', 'return', 'if', 'else', 'for', 'switch', 'case', 'default', 'break', 'continue', 'go', 'defer', 'select', 'fallthrough', 'goto', 'true', 'false', 'nil', 'int', 'int8', 'int16', 'int32', 'int64', 'uint', 'float32', 'float64', 'string', 'bool', 'byte', 'rune', 'error', 'make', 'append', 'len', 'cap', 'new', 'delete', 'close', 'panic', 'recover', 'complex64', 'complex128'],
        functions: ['fmt.Println', 'fmt.Printf', 'fmt.Sprintf', 'fmt.Fprintf', 'log.Println', 'log.Fatal', 'strings.Contains', 'strings.Split', 'strconv.Itoa', 'strconv.Atoi', 'json.Marshal', 'json.Unmarshal', 'http.Get', 'http.ListenAndServe', 'ioutil.ReadFile', 'os.Open', 'os.Exit', 'time.Now', 'time.Sleep', 'sort.Ints', 'sort.Strings', 'math.Max', 'math.Min'],
        defaultCode: `// Go Example
package main

import "fmt"

func greet(name string) {
    fmt.Printf("Hello, %s!\\n", name)
}

func main() {
    name := "World"
    greet(name)
    
    numbers := []int{1, 2, 3, 4, 5}
    sum := 0
    for _, n := range numbers {
        sum += n
    }
    fmt.Println("Sum:", sum)
}
`,
    },
    rust: {
        id: 'rust', name: 'Rust', extensions: ['.rs'],
        commentChar: '//', blockComment: ['/*', '*/'],
        keywords: ['fn', 'let', 'mut', 'const', 'static', 'struct', 'enum', 'impl', 'trait', 'type', 'pub', 'mod', 'use', 'crate', 'self', 'super', 'return', 'if', 'else', 'for', 'while', 'loop', 'match', 'break', 'continue', 'in', 'as', 'where', 'move', 'ref', 'true', 'false', 'unsafe', 'async', 'await', 'dyn', 'extern', 'macro', 'box', 'i8', 'i16', 'i32', 'i64', 'i128', 'u8', 'u16', 'u32', 'u64', 'u128', 'f32', 'f64', 'bool', 'char', 'str', 'String', 'Vec', 'Option', 'Result', 'Some', 'None', 'Ok', 'Err'],
        functions: ['println!', 'print!', 'format!', 'vec!', 'panic!', 'assert!', 'assert_eq!', 'dbg!', 'todo!', 'unimplemented!', 'push', 'pop', 'len', 'is_empty', 'contains', 'iter', 'map', 'filter', 'collect', 'unwrap', 'expect', 'clone', 'to_string', 'parse', 'from'],
        defaultCode: `// Rust Example
fn greet(name: &str) {
    println!("Hello, {}!", name);
}

fn main() {
    let name = "World";
    greet(name);
    
    let numbers = vec![1, 2, 3, 4, 5];
    let sum: i32 = numbers.iter().sum();
    println!("Sum: {}", sum);
}
`,
    },
    ruby: {
        id: 'ruby', name: 'Ruby', extensions: ['.rb'],
        commentChar: '#',
        keywords: ['def', 'end', 'class', 'module', 'if', 'elsif', 'else', 'unless', 'for', 'while', 'until', 'do', 'begin', 'rescue', 'ensure', 'raise', 'return', 'yield', 'block_given?', 'self', 'super', 'nil', 'true', 'false', 'and', 'or', 'not', 'in', 'then', 'require', 'require_relative', 'include', 'extend', 'attr_accessor', 'attr_reader', 'attr_writer', 'puts', 'print', 'p', 'gets', 'lambda', 'proc', 'case', 'when'],
        functions: ['puts', 'print', 'p', 'gets', 'chomp', 'to_s', 'to_i', 'to_f', 'length', 'size', 'each', 'map', 'select', 'reject', 'reduce', 'inject', 'sort', 'reverse', 'flatten', 'compact', 'uniq', 'push', 'pop', 'shift', 'unshift', 'include?', 'empty?', 'nil?', 'freeze', 'frozen?', 'dup', 'clone'],
        defaultCode: `# Ruby Example
def greet(name)
  puts "Hello, #{name}!"
end

numbers = [1, 2, 3, 4, 5]
sum = numbers.reduce(0, :+)
puts "Sum: #{sum}"

greet("World")
`,
    },
    php: {
        id: 'php', name: 'PHP', extensions: ['.php'],
        commentChar: '//', blockComment: ['/*', '*/'],
        keywords: ['function', 'return', 'if', 'else', 'elseif', 'for', 'foreach', 'while', 'do', 'switch', 'case', 'default', 'break', 'continue', 'class', 'extends', 'implements', 'interface', 'abstract', 'public', 'private', 'protected', 'static', 'final', 'new', 'this', 'self', 'parent', 'null', 'true', 'false', 'echo', 'print', 'var', 'const', 'use', 'namespace', 'try', 'catch', 'finally', 'throw', 'as', 'instanceof', 'array', 'string', 'int', 'float', 'bool', 'void', 'mixed', 'never', 'enum', 'match', 'fn', 'yield', 'global', 'include', 'require', 'include_once', 'require_once'],
        functions: ['echo', 'print', 'var_dump', 'print_r', 'strlen', 'strpos', 'substr', 'str_replace', 'explode', 'implode', 'array_push', 'array_pop', 'array_map', 'array_filter', 'count', 'isset', 'empty', 'unset', 'json_encode', 'json_decode', 'file_get_contents', 'date', 'time', 'intval', 'floatval', 'is_array', 'is_string', 'is_null', 'sprintf', 'preg_match', 'in_array', 'array_key_exists', 'array_merge', 'sort', 'usort', 'trim'],
        defaultCode: `<?php
// PHP Example
function greet($name) {
    echo "Hello, $name!\\n";
}

$numbers = [1, 2, 3, 4, 5];
$sum = array_sum($numbers);
echo "Sum: $sum\\n";

greet("World");
?>
`,
    },
    html: {
        id: 'html', name: 'HTML', extensions: ['.html', '.htm'],
        commentChar: '<!--',
        keywords: ['html', 'head', 'body', 'div', 'span', 'p', 'a', 'img', 'ul', 'ol', 'li', 'table', 'tr', 'td', 'th', 'form', 'input', 'button', 'select', 'option', 'textarea', 'label', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'header', 'footer', 'nav', 'main', 'section', 'article', 'aside', 'script', 'style', 'link', 'meta', 'title', 'br', 'hr', 'pre', 'code', 'strong', 'em', 'small', 'blockquote', 'iframe', 'video', 'audio', 'source', 'canvas', 'svg', 'path', 'DOCTYPE'],
        functions: ['class', 'id', 'href', 'src', 'alt', 'type', 'name', 'value', 'placeholder', 'action', 'method', 'style', 'onclick', 'onchange', 'onsubmit', 'target', 'rel', 'charset', 'content', 'width', 'height', 'data'],
        defaultCode: `<!DOCTYPE html>
<html lang="tr">
<head>
    <meta charset="UTF-8">
    <title>Merhaba Dünya</title>
</head>
<body>
    <h1>Merhaba Dünya!</h1>
    <p>Bu bir HTML sayfasıdır.</p>
    <button onclick="alert('Merhaba!')">Tıkla</button>
</body>
</html>
`,
    },
    css: {
        id: 'css', name: 'CSS', extensions: ['.css'],
        commentChar: '/*',
        keywords: ['color', 'background', 'background-color', 'background-image', 'border', 'border-radius', 'margin', 'padding', 'font-size', 'font-weight', 'font-family', 'display', 'flex', 'grid', 'position', 'top', 'right', 'bottom', 'left', 'width', 'height', 'max-width', 'min-width', 'max-height', 'min-height', 'overflow', 'z-index', 'opacity', 'transition', 'transform', 'animation', 'box-shadow', 'text-align', 'text-decoration', 'line-height', 'letter-spacing', 'cursor', 'outline', 'visibility', 'float', 'clear', 'content', 'align-items', 'justify-content', 'flex-direction', 'gap', 'order', 'flex-wrap', 'grid-template-columns', 'grid-template-rows', 'var', 'calc', 'important', 'none', 'auto', 'inherit', 'initial', 'relative', 'absolute', 'fixed', 'sticky', 'block', 'inline', 'inline-block', 'hidden', 'scroll', 'pointer', 'center', 'bold', 'normal', 'italic', 'underline', 'solid', 'dashed', 'dotted', 'ease', 'linear'],
        functions: ['rgb', 'rgba', 'hsl', 'hsla', 'url', 'calc', 'var', 'linear-gradient', 'radial-gradient', 'rotate', 'scale', 'translate', 'translateX', 'translateY', 'skew', 'perspective', 'clamp', 'min', 'max', 'minmax', 'repeat', 'fit-content', 'attr', 'counter', 'env'],
        defaultCode: `/* CSS Example */
:root {
    --primary: #3b82f6;
    --bg: #1e1e2e;
}

body {
    margin: 0;
    font-family: 'Inter', sans-serif;
    background: var(--bg);
    color: #fff;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 2rem;
    display: flex;
    gap: 1rem;
}
`,
    },
};

/* ============================================================
   THEME META — Turkish names
   ============================================================ */
export const THEME_META = {
    dark: { name: 'Gece Kadifesi', desc: 'Yumuşak koyu tonlar', group: 'Koyu' },
    light: { name: 'Gün Işığı', desc: 'Temiz ve aydınlık', group: 'Açık' },
    monokai: { name: 'Ateş Dansı', desc: 'Sıcak ve canlı renkler', group: 'Koyu' },
    dracula: { name: 'Karanlık Mor', desc: 'Mor ve pembe tonlar', group: 'Koyu' },
    nord: { name: 'Kutup Mavisi', desc: 'Soğuk kutup tonları', group: 'Koyu' },
    solarized: { name: 'Güneş Tutulması', desc: 'Bilimsel renk paleti', group: 'Koyu' },
    okyanus: { name: 'Okyanus', desc: 'Derin mavi dalgalar', group: 'Koyu' },
    gunbatimi: { name: 'Gün Batımı', desc: 'Sıcak turuncu-pembe', group: 'Koyu' },
    orman: { name: 'Orman', desc: 'Doğal yeşil huzur', group: 'Koyu' },
};

/* ============================================================
   THEME COLOR SETS (9 themes)
   ============================================================ */
const base = (o) => ({ statusBg: '#007ACC', statusText: '#ffffff', ...o });

export const themes = {
    dark: base({
        bg: '#1e1e2e', editorBg: '#181825', headerBg: '#11111b', headerBorder: '#313244',
        lineNumBg: '#181825', lineNumColor: '#585b70', lineNumActive: '#cdd6f4',
        textColor: '#cdd6f4', textMuted: '#585b70',
        consoleBg: '#11111b', consoleBorder: '#313244', consoleText: '#a6e3a1', consoleHeader: '#1e1e2e',
        sidebarBg: '#181825', sidebarBorder: '#313244',
        activityBg: '#11111b', activityIcon: '#585b70', activityActive: '#cdd6f4',
        tabBg: '#181825', tabActiveBg: '#1e1e2e', tabBorder: '#313244',
        buttonBg: '#313244', buttonHover: '#45475a',
        runBg: '#1a5c2e', runHover: '#237a3c', runText: '#a6e3a1',
        hoverBg: '#313244',
        keyword: '#569CD6', func: '#DCDCAA', string: '#CE9178', number: '#B5CEA8',
        comment: '#6A9955', operator: '#D4D4D4', punctuation: '#D4D4D4',
        preprocessor: '#C586C0', type: '#4EC9B0',
    }),
    light: base({
        bg: '#f8f9fc', editorBg: '#ffffff', headerBg: '#f0f1f5', headerBorder: '#d8dae5',
        lineNumBg: '#ffffff', lineNumColor: '#a0a4b8', lineNumActive: '#333333',
        textColor: '#24292f', textMuted: '#a0a4b8',
        consoleBg: '#f6f7fa', consoleBorder: '#d8dae5', consoleText: '#1a7f37', consoleHeader: '#edeef2',
        sidebarBg: '#f0f1f5', sidebarBorder: '#d8dae5',
        activityBg: '#e4e6ed', activityIcon: '#8b8fa3', activityActive: '#24292f',
        tabBg: '#f0f1f5', tabActiveBg: '#ffffff', tabBorder: '#d8dae5',
        buttonBg: '#e4e6ed', buttonHover: '#d0d3de',
        runBg: '#1a7f37', runHover: '#1a9642', runText: '#ffffff',
        hoverBg: '#e4e6ed',
        keyword: '#0000FF', func: '#795E26', string: '#A31515', number: '#098658',
        comment: '#008000', operator: '#000000', punctuation: '#000000',
        preprocessor: '#AF00DB', type: '#267f99',
    }),
    monokai: base({
        bg: '#2d2a2e', editorBg: '#272428', headerBg: '#221f22', headerBorder: '#403e41',
        lineNumBg: '#272428', lineNumColor: '#5b5353', lineNumActive: '#fcfcfa',
        textColor: '#fcfcfa', textMuted: '#5b5353',
        consoleBg: '#221f22', consoleBorder: '#403e41', consoleText: '#a9dc76', consoleHeader: '#2d2a2e',
        sidebarBg: '#272428', sidebarBorder: '#403e41',
        activityBg: '#221f22', activityIcon: '#5b5353', activityActive: '#fcfcfa',
        tabBg: '#272428', tabActiveBg: '#2d2a2e', tabBorder: '#403e41',
        buttonBg: '#403e41', buttonHover: '#525053',
        runBg: '#3d6a2e', runHover: '#4a8038', runText: '#a9dc76',
        statusBg: '#a9dc76', statusText: '#221f22', hoverBg: '#403e41',
        keyword: '#ff6188', func: '#a9dc76', string: '#ffd866', number: '#ab9df2',
        comment: '#727072', operator: '#ff6188', punctuation: '#939293',
        preprocessor: '#fc9867', type: '#78dce8',
    }),
    dracula: base({
        bg: '#282a36', editorBg: '#21222c', headerBg: '#191a21', headerBorder: '#44475a',
        lineNumBg: '#21222c', lineNumColor: '#6272a4', lineNumActive: '#f8f8f2',
        textColor: '#f8f8f2', textMuted: '#6272a4',
        consoleBg: '#191a21', consoleBorder: '#44475a', consoleText: '#50fa7b', consoleHeader: '#282a36',
        sidebarBg: '#21222c', sidebarBorder: '#44475a',
        activityBg: '#191a21', activityIcon: '#6272a4', activityActive: '#bd93f9',
        tabBg: '#21222c', tabActiveBg: '#282a36', tabBorder: '#44475a',
        buttonBg: '#44475a', buttonHover: '#6272a4',
        runBg: '#2d6a3a', runHover: '#3a8a4e', runText: '#50fa7b',
        statusBg: '#bd93f9', statusText: '#282a36', hoverBg: '#44475a',
        keyword: '#ff79c6', func: '#50fa7b', string: '#f1fa8c', number: '#bd93f9',
        comment: '#6272a4', operator: '#ff79c6', punctuation: '#f8f8f2',
        preprocessor: '#ffb86c', type: '#8be9fd',
    }),
    nord: base({
        bg: '#2e3440', editorBg: '#2a303c', headerBg: '#242933', headerBorder: '#3b4252',
        lineNumBg: '#2a303c', lineNumColor: '#4c566a', lineNumActive: '#d8dee9',
        textColor: '#d8dee9', textMuted: '#4c566a',
        consoleBg: '#242933', consoleBorder: '#3b4252', consoleText: '#a3be8c', consoleHeader: '#2e3440',
        sidebarBg: '#2a303c', sidebarBorder: '#3b4252',
        activityBg: '#242933', activityIcon: '#4c566a', activityActive: '#88c0d0',
        tabBg: '#2a303c', tabActiveBg: '#2e3440', tabBorder: '#3b4252',
        buttonBg: '#3b4252', buttonHover: '#434c5e',
        runBg: '#304830', runHover: '#3d5c3d', runText: '#a3be8c',
        statusBg: '#5e81ac', statusText: '#eceff4', hoverBg: '#3b4252',
        keyword: '#81a1c1', func: '#88c0d0', string: '#a3be8c', number: '#b48ead',
        comment: '#616e88', operator: '#81a1c1', punctuation: '#d8dee9',
        preprocessor: '#d08770', type: '#8fbcbb',
    }),
    solarized: base({
        bg: '#002b36', editorBg: '#073642', headerBg: '#002028', headerBorder: '#0a4a5c',
        lineNumBg: '#073642', lineNumColor: '#586e75', lineNumActive: '#93a1a1',
        textColor: '#839496', textMuted: '#586e75',
        consoleBg: '#002028', consoleBorder: '#0a4a5c', consoleText: '#859900', consoleHeader: '#002b36',
        sidebarBg: '#073642', sidebarBorder: '#0a4a5c',
        activityBg: '#002028', activityIcon: '#586e75', activityActive: '#268bd2',
        tabBg: '#073642', tabActiveBg: '#002b36', tabBorder: '#0a4a5c',
        buttonBg: '#0a4a5c', buttonHover: '#1a6a7c',
        runBg: '#2a5a00', runHover: '#3a7a10', runText: '#859900',
        statusBg: '#268bd2', statusText: '#fdf6e3', hoverBg: '#0a4a5c',
        keyword: '#268bd2', func: '#b58900', string: '#2aa198', number: '#d33682',
        comment: '#586e75', operator: '#93a1a1', punctuation: '#93a1a1',
        preprocessor: '#cb4b16', type: '#859900',
    }),
    okyanus: base({
        bg: '#0f1b2d', editorBg: '#0a1628', headerBg: '#071020', headerBorder: '#1a3050',
        lineNumBg: '#0a1628', lineNumColor: '#2a4a6a', lineNumActive: '#7ec8e3',
        textColor: '#c8dce8', textMuted: '#2a4a6a',
        consoleBg: '#071020', consoleBorder: '#1a3050', consoleText: '#56d6a0', consoleHeader: '#0f1b2d',
        sidebarBg: '#0a1628', sidebarBorder: '#1a3050',
        activityBg: '#071020', activityIcon: '#2a4a6a', activityActive: '#7ec8e3',
        tabBg: '#0a1628', tabActiveBg: '#0f1b2d', tabBorder: '#1a3050',
        buttonBg: '#1a3050', buttonHover: '#264a70',
        runBg: '#1a5040', runHover: '#207050', runText: '#56d6a0',
        statusBg: '#1a6090', statusText: '#e0f0ff', hoverBg: '#1a3050',
        keyword: '#7ec8e3', func: '#56d6a0', string: '#f0c070', number: '#c098e0',
        comment: '#3a6080', operator: '#7ec8e3', punctuation: '#5a8aaa',
        preprocessor: '#e0a050', type: '#56d6a0',
    }),
    gunbatimi: base({
        bg: '#1f1520', editorBg: '#1a1018', headerBg: '#150c14', headerBorder: '#3a2535',
        lineNumBg: '#1a1018', lineNumColor: '#5a3a50', lineNumActive: '#f0c8a0',
        textColor: '#e8d0c0', textMuted: '#5a3a50',
        consoleBg: '#150c14', consoleBorder: '#3a2535', consoleText: '#a0e080', consoleHeader: '#1f1520',
        sidebarBg: '#1a1018', sidebarBorder: '#3a2535',
        activityBg: '#150c14', activityIcon: '#5a3a50', activityActive: '#f0a060',
        tabBg: '#1a1018', tabActiveBg: '#1f1520', tabBorder: '#3a2535',
        buttonBg: '#3a2535', buttonHover: '#4a3545',
        runBg: '#4a3020', runHover: '#5a4030', runText: '#f0c070',
        statusBg: '#c06040', statusText: '#fff0e0', hoverBg: '#3a2535',
        keyword: '#f07060', func: '#f0c070', string: '#e0a0d0', number: '#80c8f0',
        comment: '#6a4a5a', operator: '#f0a060', punctuation: '#8a6a7a',
        preprocessor: '#f08050', type: '#60d0b0',
    }),
    orman: base({
        bg: '#1a2418', editorBg: '#152010', headerBg: '#0e180c', headerBorder: '#2a3a25',
        lineNumBg: '#152010', lineNumColor: '#3a5a35', lineNumActive: '#b0d8a0',
        textColor: '#c8e0c0', textMuted: '#3a5a35',
        consoleBg: '#0e180c', consoleBorder: '#2a3a25', consoleText: '#80d860', consoleHeader: '#1a2418',
        sidebarBg: '#152010', sidebarBorder: '#2a3a25',
        activityBg: '#0e180c', activityIcon: '#3a5a35', activityActive: '#80d860',
        tabBg: '#152010', tabActiveBg: '#1a2418', tabBorder: '#2a3a25',
        buttonBg: '#2a3a25', buttonHover: '#3a4a35',
        runBg: '#2a4a20', runHover: '#3a5a30', runText: '#80d860',
        statusBg: '#3a7030', statusText: '#e0f8d8', hoverBg: '#2a3a25',
        keyword: '#70b8e0', func: '#b0d860', string: '#e0c080', number: '#c0a0e0',
        comment: '#4a6a45', operator: '#90c870', punctuation: '#6a8a60',
        preprocessor: '#d8a050', type: '#60c8a0',
    }),
};

/* ============================================================
   PREVIEW CODE (for theme selector)
   ============================================================ */
export const PREVIEW_CODE = `tamsayı ana() {
    tamsayı x = 42;
    çıktı("Merhaba!");
    eğer (x > 10) {
        dön x;
    }
}`;

/* ============================================================
   DETECT LANGUAGE FROM FILE NAME
   ============================================================ */
export function detectLanguage(fileName) {
    if (!fileName) return 'turkce_c';
    const lower = fileName.toLowerCase();
    // Check longest extensions first (e.g. .py.tr before .py)
    const sorted = Object.entries(LANGUAGES).sort((a, b) => {
        const maxExtA = Math.max(...a[1].extensions.map(e => e.length));
        const maxExtB = Math.max(...b[1].extensions.map(e => e.length));
        return maxExtB - maxExtA;
    });
    for (const [id, lang] of sorted) {
        for (const ext of lang.extensions) {
            if (lower.endsWith(ext)) return id;
        }
    }
    return 'turkce_c';
}

/* ============================================================
   FILE ICON HELPER
   ============================================================ */
export function getFileIcon(fileName) {
    const lang = detectLanguage(fileName);
    const iconMap = {
        javascript: '🟨', typescript: '🔷', python: '🐍',
        java: '☕', cpp: '⚙️', csharp: '🟪', go: '🔵',
        rust: '🦀', ruby: '💎', php: '🐘', html: '🌐', css: '🎨',
        turkce_c: '🇹🇷', turkce_python: '🐍',
    };
    return iconMap[lang] || '📄';
}

/* ============================================================
   INTELLISENSE HELPER
   ============================================================ */
export function getSuggestions(langId, prefix) {
    if (!prefix || prefix.length < 1) return [];
    const lang = LANGUAGES[langId] || LANGUAGES.turkce_c;
    const lower = prefix.toLowerCase();

    // Combine keywords and functions
    const all = [
        ...lang.keywords.map(k => ({ text: k, type: 'keyword' })),
        ...lang.functions.map(f => ({ text: f, type: 'function' }))
    ];

    // Filter and sort
    return all
        .filter(item => item.text.toLowerCase().startsWith(lower))
        .sort((a, b) => a.text.localeCompare(b.text))
        .slice(0, 10); // Limit to top 10
}

/* ============================================================
   SYNTAX HIGHLIGHTER (universal)
   ============================================================ */
function escapeHtml(text) {
    return text.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
}

export function highlightLine(line, themeKey, langId) {
    const t = themes[themeKey];
    if (!t) return escapeHtml(line);
    const lang = LANGUAGES[langId || 'turkce_c'] || LANGUAGES.turkce_c;
    let result = '';
    let i = 0;

    const cc = lang.commentChar;
    // Find comment start position
    let commentIdx = -1;
    if (cc === '#' || cc === '//') {
        let inStr = false, sq = null;
        for (let ci = 0; ci < line.length; ci++) {
            const c = line[ci];
            if ((c === '"' || c === "'") && (ci === 0 || line[ci - 1] !== '\\')) {
                if (!inStr) { inStr = true; sq = c; }
                else if (c === sq) { inStr = false; }
            }
            if (!inStr) {
                if (cc === '//' && c === '/' && ci + 1 < line.length && line[ci + 1] === '/') { commentIdx = ci; break; }
                if (cc === '#' && c === '#') { commentIdx = ci; break; }
            }
        }
    }
    if (cc === '<!--') {
        const hci = line.indexOf('<!--');
        if (hci !== -1) commentIdx = hci;
    }
    if (cc === '/*') {
        const bci = line.indexOf('/*');
        if (bci !== -1) commentIdx = bci;
    }

    while (i < line.length) {
        // Comment
        if (commentIdx !== -1 && i === commentIdx) {
            result += `<span style="color:${t.comment};font-style:italic">${escapeHtml(line.slice(i))}</span>`;
            break;
        }

        // Preprocessor (C-like)
        if ((langId === 'turkce_c' || langId === 'cpp') && i === 0) {
            const preMatch = line.match(/^(dahil_et|tanım|#\s*include|#\s*define|#\s*ifdef|#\s*ifndef|#\s*endif|#\s*pragma)\b/);
            if (preMatch) {
                result += `<span style="color:${t.preprocessor};font-weight:600">${escapeHtml(preMatch[0])}</span>`;
                i += preMatch[0].length;
                const rest = line.slice(i);
                const angleMatch = rest.match(/^(\s*<[^>]*>)/);
                if (angleMatch) {
                    result += `<span style="color:${t.string}">${escapeHtml(angleMatch[0])}</span>`;
                    i += angleMatch[0].length;
                }
                continue;
            }
        }

        // HTML tags
        if (langId === 'html' && line[i] === '<') {
            const tagMatch = line.slice(i).match(/^(<\/?[a-zA-Z][a-zA-Z0-9]*)/);
            if (tagMatch) {
                result += `<span style="color:${t.keyword}">${escapeHtml(tagMatch[0])}</span>`;
                i += tagMatch[0].length;
                // Attributes
                while (i < line.length && line[i] !== '>') {
                    const attrMatch = line.slice(i).match(/^(\s+[a-zA-Z\-]+)/);
                    if (attrMatch) {
                        result += `<span style="color:${t.func}">${escapeHtml(attrMatch[0])}</span>`;
                        i += attrMatch[0].length;
                        continue;
                    }
                    if (line[i] === '=' || line[i] === '"' || line[i] === "'") {
                        if (line[i] === '=') {
                            result += escapeHtml('=');
                            i++;
                        }
                        if (line[i] === '"' || line[i] === "'") {
                            const q = line[i];
                            let j = i + 1;
                            while (j < line.length && line[j] !== q) j++;
                            j = Math.min(j + 1, line.length);
                            result += `<span style="color:${t.string}">${escapeHtml(line.slice(i, j))}</span>`;
                            i = j;
                        }
                        continue;
                    }
                    result += escapeHtml(line[i]);
                    i++;
                }
                if (i < line.length && line[i] === '>') {
                    const closing = line[i - 1] === '/' ? '/>' : '>';
                    result += `<span style="color:${t.keyword}">${escapeHtml(line[i])}</span>`;
                    i++;
                }
                continue;
            }
        }

        // CSS selectors/properties
        if (langId === 'css') {
            // Property names (before colon)
            const cssProp = line.slice(i).match(/^([a-z\-]+)(\s*:)/);
            if (cssProp && i > 0) {
                result += `<span style="color:${t.keyword}">${escapeHtml(cssProp[1])}</span>`;
                result += `<span style="color:${t.punctuation}">${escapeHtml(cssProp[2])}</span>`;
                i += cssProp[0].length;
                continue;
            }
            // Selectors (., #, :, @)
            const selMatch = line.slice(i).match(/^([.#@:][a-zA-Z_\-][a-zA-Z0-9_\-]*)/);
            if (selMatch) {
                result += `<span style="color:${t.func}">${escapeHtml(selMatch[0])}</span>`;
                i += selMatch[0].length;
                continue;
            }
        }

        // Python/Ruby decorator
        if ((langId === 'python' || langId === 'turkce_python' || langId === 'ruby') && line.slice(i).match(/^@\w+/)) {
            const dm = line.slice(i).match(/^@\w+/);
            result += `<span style="color:${t.preprocessor}">${escapeHtml(dm[0])}</span>`;
            i += dm[0].length;
            continue;
        }

        // Template literals for JS/TS
        if ((langId === 'javascript' || langId === 'typescript') && line[i] === '`') {
            let j = i + 1;
            while (j < line.length && line[j] !== '`') j++;
            j = Math.min(j + 1, line.length);
            result += `<span style="color:${t.string}">${escapeHtml(line.slice(i, j))}</span>`;
            i = j;
            continue;
        }

        // Strings
        if (line[i] === '"' || line[i] === "'") {
            const q = line[i];
            let j = i + 1;
            while (j < line.length && (line[j] !== q || line[j - 1] === '\\')) j++;
            j = Math.min(j + 1, line.length);
            result += `<span style="color:${t.string}">${escapeHtml(line.slice(i, j))}</span>`;
            i = j;
            continue;
        }

        // Numbers
        const numMatch = line.slice(i).match(/^\b\d+(\.\d+)?\b/);
        if (numMatch && (i === 0 || /[\s(,=+\-*/<>!&|;:{}[\]%]/.test(line[i - 1]))) {
            result += `<span style="color:${t.number}">${escapeHtml(numMatch[0])}</span>`;
            i += numMatch[0].length;
            continue;
        }

        // Words
        const wordMatch = line.slice(i).match(/^[a-zA-ZçÇğĞıİöÖşŞüÜ_$][a-zA-Z0-9çÇğĞıİöÖşŞüÜ_$]*/);
        if (wordMatch) {
            const word = wordMatch[0];
            if (lang.keywords.includes(word)) {
                result += `<span style="color:${t.keyword};font-weight:700">${escapeHtml(word)}</span>`;
            } else if (lang.functions.includes(word)) {
                result += `<span style="color:${t.func}">${escapeHtml(word)}</span>`;
            } else {
                const afterWord = line.slice(i + word.length);
                if (/^\s*\(/.test(afterWord)) {
                    result += `<span style="color:${t.func}">${escapeHtml(word)}</span>`;
                } else {
                    result += `<span style="color:${t.textColor}">${escapeHtml(word)}</span>`;
                }
            }
            i += word.length;
            continue;
        }

        // Operators
        const opMatch = line.slice(i).match(/^(===|!==|==|!=|<=|>=|&&|\|\||<<|>>|=>|\+\+|--|->|\?\?|\?\.|\.\.\.|[+\-*/%=<>!&|^~?])/);
        if (opMatch) {
            result += `<span style="color:${t.operator}">${escapeHtml(opMatch[0])}</span>`;
            i += opMatch[0].length;
            continue;
        }

        // Punctuation
        if ('(){}[];,.:#@'.includes(line[i])) {
            result += `<span style="color:${t.punctuation}">${escapeHtml(line[i])}</span>`;
            i++;
            continue;
        }

        result += escapeHtml(line[i]);
        i++;
    }
    return result;
}

/* ============================================================
   SYNTAX VALIDATOR (universal)
   ============================================================ */
/* ============================================================
   SYNTAX VALIDATOR (universal)
   ============================================================ */
export function validateSyntax(code, langId) {
    const lang = LANGUAGES[langId];
    if (!lang) return [];
    if (langId === 'turkce_python' || langId === 'python') return validatePython(code.split('\n'), langId);
    if (langId === 'html' || langId === 'css') return []; // skip for markup
    return validateCLike(code.split('\n'), langId);
}

function validateCLike(lines, langId) {
    const errors = [];
    const stack = [];
    const lang = LANGUAGES[langId];
    const commentChar = lang?.commentChar || '//';

    for (let lineIdx = 0; lineIdx < lines.length; lineIdx++) {
        const line = lines[lineIdx];
        const trimmed = line.trim();
        const lineNum = lineIdx + 1;

        if (lineIdx === 0 && trimmed.startsWith('#')) continue; // Shebang or similar
        if (lines[lineIdx] === undefined) continue;

        if (longComment(trimmed)) continue; // Simplified check

        if (trimmed === '' || trimmed.startsWith(commentChar) || trimmed.startsWith('/*')) continue;
        if (trimmed.startsWith('#') || trimmed.startsWith('dahil_et') || trimmed.startsWith('import') || trimmed.startsWith('package') || trimmed.startsWith('using')) continue;

        // 1. Bracket Matching
        let inString = false, stringChar = null;
        for (let i = 0; i < line.length; i++) {
            const ch = line[i];
            const prev = i > 0 ? line[i - 1] : '';
            if ((ch === '"' || ch === "'" || ch === '`') && prev !== '\\') {
                if (!inString) { inString = true; stringChar = ch; }
                else if (ch === stringChar) { inString = false; stringChar = null; }
                continue;
            }
            if (inString) continue;
            if (ch === '/' && i + 1 < line.length && line[i + 1] === '/') break;
            if ('({['.includes(ch)) stack.push({ char: ch, line: lineNum });
            else if (')}]'.includes(ch)) {
                const expected = ch === ')' ? '(' : ch === '}' ? '{' : '[';
                if (stack.length === 0) {
                    errors.push({ line: lineNum, message: `Fazla kapanış '${ch}'`, severity: 'error' });
                } else if (stack[stack.length - 1].char !== expected) {
                    const top = stack[stack.length - 1];
                    errors.push({ line: lineNum, message: `'${ch}' ile eşleşmeyen '${top.char}' (satır ${top.line})`, severity: 'error' });
                } else { stack.pop(); }
            }
        }
        if (inString) errors.push({ line: lineNum, message: `Kapanmamış metin dizisi`, severity: 'error' });

        // 2. Semicolon Check (for non-block lines)
        const noSemiKeywords = ['if', 'else', 'for', 'while', 'switch', 'case', 'default', 'eğer', 'değilse', 'döngü', 'süre', 'yap_süre'];
        const firstWord = trimmed.split(/[\s(]/)[0];

        if (['turkce_c', 'cpp', 'java', 'csharp', 'php'].includes(langId) &&
            !trimmed.endsWith('{') && !trimmed.endsWith('}') && !trimmed.endsWith(';') && !trimmed.endsWith(':') &&
            !trimmed.startsWith('//') && !noSemiKeywords.includes(firstWord)) {
            errors.push({ line: lineNum, message: `Satır sonuna ';' bekleniyor`, severity: 'warning' });
        }
    }
    while (stack.length > 0) {
        const unclosed = stack.pop();
        errors.push({ line: unclosed.line, message: `Açılan '${unclosed.char}' hiç kapatılmadı`, severity: 'error' });
    }
    return errors;
}

function longComment(str) { return str.startsWith('/*') || str.endsWith('*/'); }

function validatePython(lines, langId) {
    const errors = [];
    const blockKeywords = langId === 'turkce_python'
        ? ['tanımla', 'sınıf', 'eğer', 'değilse_eğer', 'değilse', 'için', 'süre', 'dene', 'yakala', 'sonunda', 'ile']
        : ['def', 'class', 'if', 'elif', 'else', 'for', 'while', 'try', 'except', 'finally', 'with'];
    for (let lineIdx = 0; lineIdx < lines.length; lineIdx++) {
        const line = lines[lineIdx];
        const trimmed = line.trim();
        const lineNum = lineIdx + 1;
        if (trimmed === '' || trimmed.startsWith('#')) continue;

        let inString = false, stringChar = null;
        for (let i = 0; i < line.length; i++) {
            const ch = line[i];
            const prev = i > 0 ? line[i - 1] : '';
            if ((ch === '"' || ch === "'") && prev !== '\\') {
                if (!inString) { inString = true; stringChar = ch; }
                else if (ch === stringChar) { inString = false; }
            }
            if (!inString && ch === '#') break;
        }
        if (inString) errors.push({ line: lineNum, message: `Kapanmamış metin dizisi` });

        for (const kw of blockKeywords) {
            if (trimmed.startsWith(kw + ' ') || trimmed === kw || trimmed.startsWith(kw + '(') ||
                (kw === 'else' || kw === 'değilse') && trimmed === kw) {
                if (!trimmed.endsWith(':')) {
                    errors.push({ line: lineNum, message: `'${kw}' ifadesinden sonra ':' eksik`, severity: 'warning' });
                }
                break;
            }
        }
    }
    return errors;
}

/* ============================================================
   RUN SIMULATOR (universal)
   ============================================================ */
/* ============================================================
   RUN SIMULATOR (universal)
   ============================================================ */
export function simulateRun(code, langId) {
    try {
        if (langId === 'turkce_python') return simulatePython(code, true);
        if (langId === 'python') return simulatePython(code, false);
        if (langId === 'turkce_c') return simulateC(code);
        if (langId === 'javascript' || langId === 'typescript') return simulateJS(code);
        return simulateGeneric(code, langId);
    } catch (e) {
        return [`❌ Çalışma Zamanı Hatası: ${e.message}`];
    }
}

function simulateC(code) {
    const output = [];
    const vars = {};
    const lines = code.split('\n');

    for (const line of lines) {
        const trimmed = line.trim();
        if (!trimmed || trimmed.startsWith('//')) continue;

        // Declaration & Assignment: tamsayı x = 10;
        const declMatch = trimmed.match(/^(?:tamsayı|ondalık|çift|metin)\s+(\w+)\s*=\s*([^;]+);/);
        if (declMatch) {
            const varName = declMatch[1];
            const expr = declMatch[2].trim();
            vars[varName] = evalExpr(expr, vars);
            continue;
        }

        // Reassignment
        const assignMatch = trimmed.match(/^(\w+)\s*=\s*([^;]+);/);
        if (assignMatch) {
            const varName = assignMatch[1];
            if (vars[varName] === undefined) throw new Error(`'${varName}' tanımlanmamış değişken`);
            vars[varName] = evalExpr(assignMatch[2], vars);
            continue;
        }

        // Output: çıktı("...", ...);
        const printMatch = trimmed.match(/çıktı\(([^)]*)\)/);
        if (printMatch) {
            const content = printMatch[1].trim();
            if (!content) { output.push(""); continue; }
            const parts = splitArgs(content);
            let fmt = parts[0];
            const args = parts.slice(1).map(a => evalExpr(a, vars));

            if ((fmt.startsWith('"') && fmt.endsWith('"'))) {
                fmt = fmt.slice(1, -1);
                let argIdx = 0;
                let text = fmt.replace(/%[dsf]/g, () => {
                    return argIdx < args.length ? args[argIdx++] : '%?';
                }).replace(/\\n/g, '');
                output.push(text);
            } else {
                output.push(String(evalExpr(fmt, vars)));
            }
        }
    }
    return output;
}

// Helpers
function evalExpr(expr, vars) {
    expr = String(expr).trim();
    if (!isNaN(expr)) return parseFloat(expr);
    if ((expr.startsWith('"') && expr.endsWith('"'))) return expr.slice(1, -1);
    if (vars[expr] !== undefined) return vars[expr];

    const math = expr.match(/^(.+)\s*([\+\-\*\/])\s*(.+)$/);
    if (math) {
        const left = evalExpr(math[1], vars);
        const right = evalExpr(math[3], vars);
        const op = math[2];
        if (op === '+') return left + right;
        if (op === '-') return left - right;
        if (op === '*') return left * right;
        if (op === '/') {
            if (right === 0) throw new Error("Sıfıra bölme hatası");
            return left / right;
        }
    }
    return expr;
}

function splitArgs(str) {
    const result = [];
    let current = '';
    let inQuote = false;
    for (let i = 0; i < str.length; i++) {
        const c = str[i];
        if (c === '"') inQuote = !inQuote;
        if (c === ',' && !inQuote) {
            result.push(current.trim());
            current = '';
        } else {
            current += c;
        }
    }
    result.push(current.trim());
    return result;
}

function simulatePython(code, isTurkish) {
    const output = [];
    const vars = {};
    const printFn = isTurkish ? 'yazdır' : 'print';

    const lines = code.split('\n');
    for (const line of lines) {
        const trimmed = line.trim();
        if (!trimmed || trimmed.startsWith('#')) continue;

        const assignMatch = trimmed.match(/^(\w+)\s*=\s*(.+)$/);
        if (assignMatch && !trimmed.startsWith(printFn)) {
            const varName = assignMatch[1];
            try {
                vars[varName] = evalPyExpr(assignMatch[2], vars);
            } catch (e) { throw new Error(`${e.message}`); }
            continue;
        }

        if (trimmed.startsWith(printFn + '(')) {
            const content = trimmed.slice(printFn.length + 1, -1);
            output.push(String(evalPyExpr(content, vars)));
        }
    }
    return output;
}

function evalPyExpr(expr, vars) {
    let result = expr;
    // Sort keys by length desc to avoid replacing substrings of other vars
    Object.keys(vars).sort((a, b) => b.length - a.length).forEach(v => {
        const regex = new RegExp(`\\b${v}\\b`, 'g');
        if (result.match(regex) && !result.startsWith('"') && !result.startsWith("'")) {
            result = result.replace(regex, typeof vars[v] === 'string' ? `"${vars[v]}"` : vars[v]);
        }
    });

    if (result.startsWith('f"') || result.startsWith("f'")) {
        return result.slice(2, -1).replace(/{(\w+)}/g, (_, v) => vars[v] ?? `{${v}}`);
    }

    try {
        // eslint-disable-next-line no-eval
        return eval(result);
    } catch {
        return result.replace(/^["']|["']$/g, '');
    }
}

function simulateJS(code) {
    const output = [];
    const context = {
        console: { log: (...args) => output.push(args.join(' ')) },
        setTimeout: () => { }, setInterval: () => { }
    };
    try {
        const fn = new Function('console', code);
        fn(context.console);
    } catch (e) {
        throw new Error(e.message);
    }
    return output;
}

function simulateGeneric(code, langId) {
    const output = [];
    const lang = LANGUAGES[langId];
    if (!lang) return output;
    const printPatterns = [
        /System\.out\.println\((.+)\)/,
        /fmt\.Println\((.+)\)/,
        /println!\((.+)\)/,
        /puts\s+(.+)/,
        /echo\s+(.+)/,
        /Console\.WriteLine\((.+)\)/,
    ];
    for (const line of code.split('\n')) {
        const trimmed = line.trim();
        for (const pat of printPatterns) {
            const m = trimmed.match(pat);
            if (m) {
                let val = m[1].trim();
                if ((val.startsWith('"') && val.endsWith('"')) || (val.startsWith("'") && val.endsWith("'"))) val = val.slice(1, -1);
                val = val.replace(/\\n/g, '');
                if (val) output.push(val);
                break;
            }
        }
    }
    return output;
}
