# Problema das n damas

O problema das n damas é o problema matemático de dispor n damas em um tabuleiro de xadrez de dimensão n x n, de forma que nenhuma delas seja atacada por outra. Para tanto, é necessário que duas damas quaisquer não estejam numa mesma linha, coluna, ou diagonal. Este é um caso específico do Problema das n damas, no qual temos n damas e um tabuleiro com n×n casas(para qualquer n  ≥ 4).

## Rejeitando soluções equivalentes

Você pode restringir as soluções aceitáveis ​​às soluções exclusivas. Algumas soluções possuem o que chamamos de simetria rotacional: se você girar a placa em 180o, ou talvez até 90o, você terá exatamente a mesma configuração. Abaixo é demonstrado duas soluções, uma com simetria rotacional de 180o e outra com simetria rotacional de 90o. A numeração na figura mostra as rainhas que são equivalentes na rotação.
```
      .  1  .  .  .  .                .  1  .  .  .
      .  .  .  2  .  .                .  .  .  .  1
      .  .  .  .  .  3                .  .  2  .  .
      3  .  .  .  .  .                1  .  .  .  .
      .  .  2  .  .  .                .  .  .  1  .
      .  .  .  .  1  .         Symmetric on 90o rotation
Symmetric on 180o rotation
```
Se uma solução não possuir essa simetria rotacional, rotações sucessivas a 90o gerarão outras soluções que serão descobertas à medida que você processa as soluções. Além das rotações, há outra operação de simetria: reflexão em um espelho. Pela própria natureza do problema das rainhas N, uma solução válida não pode ter uma simetria de espelho. Isso significa que cada solução válida também possui imagens espelhadas que serão exibidas no processamento. A Figura 3 mostra a solução para o problema do 5-Queens que não possui simetria rotacional (a Figura 2 mostra a solução simétrica). Consequentemente, existem oito soluções equivalentes, sendo simplesmente rotações ou reflexos de uma configuração inicial da placa.

Você pode pensar que rejeitar soluções equivalentes exigiria a pesquisa de soluções aceitas anteriormente, mas existe uma abordagem alternativa que não exige salvar as soluções anteriores. Você está representando o quadro por uma matriz de posições de coluna. Tudo o que você precisa fazer é considerar a ordenação lexicográfica das soluções, pensando na matriz como um número de N dígitos. Se você tem a regra de que só aceitará a primeira solução nesse pedido de soluções equivalentes, a rejeição de todo o resto é direta. Para uma solução candidata, gire-a em incrementos sucessivos de 90o. Se a qualquer momento o resultado for comparado como "menor", rejeite a solução candidata. Para as imagens espelhadas, você pode gerar uma imagem espelhada e, em seguida, girá-la através de três incrementos sucessivos de 90o para verificar as quatro imagens espelhadas.

       Original                       Vertical mirror
         1  .  .  .  .                  .  .  .  .  1
         .  .  2  .  .                  .  .  2  .  .
         .  .  .  .  3                  3  .  .  .  .
         .  4  .  .  .                  .  .  .  4  .
         .  .  .  5  .                  .  5  .  .  .

       90 degree rotation             Anti-diagonal mirror
         .  .  .  .  1                  .  .  3  .  .
         .  4  .  .  .                  5  .  .  .  .
         .  .  .  2  .                  .  .  .  2  .
         5  .  .  .  .                  .  4  .  .  .
         .  .  3  .  .                  .  .  .  .  1

       180 degree rotation            Horizontal mirror
         .  5  .  .  .                  .  .  .  5  .
         .  .  .  4  .                  .  4  .  .  .
         3  .  .  .  .                  .  .  .  .  3
         .  .  2  .  .                  .  .  2  .  .
         .  .  .  .  1                  1  .  .  .  .

       270 degree rotation            Diagonal mirror
         .  .  3  .  .                  1  .  .  .  .
         .  .  .  .  5                  .  .  .  4  .
         .  2  .  .  .                  .  2  .  .  .
         .  .  .  4  .                  .  .  .  .  5
         1  .  .  .  .                  .  .  3  .  .

# Soluções de contagem
As tabelas a seguir fornecem o número de soluções para colocar n rainhas em uma tabuleiro de n × n, fundamentais e todas

| n	| Fundamentais	| Todas|
----|---------------|------
| 1	 | 1	| 1|
| 2	 | 0	| 0|
| 3	 | 0	| 0
| 4	 | 1	| 2
| 5	 | 2	| 10
| 6	 | 1	| 4
| 7	 | 6	| 40
| 8	 | 12	| 92
| 9	 | 46	| 352
| 10 | 	92	| 724
| 11 | 	341	| 2,680
| 12 | 	1,787	|14,200
| 13 | 	9,233	|73,712
| 14 | 	45,752 | 365,596
| 15 | 	285,053	| 2,279,184
| 16 | 	1,846,955	| 14,772,512
| 17 | 	11,977,939	| 95,815,104
| 18 | 	83,263,591	| 666,090,624


## Compilação

Estando dentro do diretório correto basta executar o *Makefile*.

> $ make

Após execução deste comando os códigos serão compilados e gerados os executáveis e binarios.

## Excecução

Para executar o modo sequencial, utilize a função abaixo com o primeiro parâmetro o número de rainhas.
> $ ./seq-nqueens 12

Já para o paralelo, utilize verifique o arquivo `mp` utilizado para o `--hostfile`

> $ mpirun -np 8 --hostfile mp ./mpi-nqueens 12

## Referências:
* http://penguin.ewu.edu/~trolfe/Queens/OptQueen.html
