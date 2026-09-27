# Bits, bytes e bases numéricas

Um bit possui dois estados. O significado de uma sequência de bits não está contido nos bits: depende da convenção usada para interpretá-los.

`11111111₂` pode significar 255 como unsigned de 8 bits, -1 em complemento de dois, uma parte de uma cor, um byte de uma instrução ou dados compactados.

## Conversão

Em base 2:

`101101₂ = 1·2⁵ + 0·2⁴ + 1·2³ + 1·2² + 0·2¹ + 1·2⁰ = 45`.

Hexadecimal agrupa quatro bits por dígito:

~~~text
1011 1100
  B    C
= 0xBC
~~~

Isso torna endereços, máscaras e bytes mais legíveis.

## Operações bitwise

- `x & mask`: seleciona bits;
- `x | mask`: liga bits;
- `x ^ mask`: alterna bits;
- `~x`: inverte;
- `x << n`: desloca à esquerda;
- `x >> n`: desloca à direita.

Exemplo:

~~~python
READ = 0b001
WRITE = 0b010
EXECUTE = 0b100

permissions = READ | WRITE
can_write = bool(permissions & WRITE)
~~~

Uma única palavra inteira representa vários flags independentes.

## Overflow

Com 8 bits unsigned, o intervalo é 0..255. Se o hardware conservar apenas oito bits:

`255 + 1 = 256 = 1_0000_0000₂ → 0000_0000₂`.

O resultado matemático não “virou zero”; o registrador não possui espaço para o nono bit.

Esse modelo será usado em ponteiros, ALU, protocolos, criptografia, arquivos e bancos de dados.
