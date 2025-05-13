# 🧿 PGCCHIB - Processamento Gráfico (2025/1)

Repositório com as soluções dos exercícios da disciplina **Processamento Gráfico** (2025/1), compilado em **Pop!_OS 22.04 LTS**.

Trabalhos feitos por: **Otávio Henrique**

## 📌 Atividades

### Modulo 2

**Parte 1 – Sem matriz de transformação**
- **Ex. 1:** Função `createTriangle(x0, y0, x1, y1, x2, y2)` que retorna o VAO de um triângulo.
- **Ex. 2:** Instanciação de 5 triângulos usando a função anterior, armazenados em vetor e renderizados no loop principal.

**Parte 2 – Com matriz de transformação (GLM)**
- **Ex. 3:** Estrutura `Triangle` com posição (x, y) e cor (RGB).
    - Um VAO base com triângulo padrão.
    - Novos triângulos criados com cliques do mouse e cores sorteadas.

## 📁 Estrutura

```
PGCCHIB/
├── include/         # GLAD e dependências
├── common/          # glad.c
├── src/             # Códigos dos exercícios
├── CMakeLists.txt
```

## ⚠️ GLAD manual
Coloque os arquivos gerados em:
- `include/glad/`
- `include/glad/KHR/`
- `common/glad.c`

## ⚙️ Compilação (via CMake)

```bash
mkdir build
cd build
cmake ..
make
./nome_do_exec
```

## 📚 Exercícios Disponíveis
- `m2_p1`: Implementa os **Exercícios 1 e 2** do **Módulo 2** (sem matriz de transformação).
- `m2_p2`: Implementa o **Exercício 3** do **Módulo 2** (com matriz de transformação, uso de GLM).
