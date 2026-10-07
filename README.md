# GBV - Gerenciador de Biblioteca Virtual

Implementação em C de um gerenciador de arquivos para a disciplina de Programação 2 cursada em 2026.1 na Universidade Federal do Paraná.


## Funcionalidades
- Insere documentos na biblioteca, caso seja inserido um arquivo com o mesmo nome de um que já existente na biblioteca, este é sobrescrevido (-a).
- Remove os metadados dos arquivos da biblioteca (-r).
- Vizualiza em blocos o conteúdo dos arquivos da biblioteca, navegando os blocos usando 'n' (proximo bloco), 'p'(bloco anterior) e 'q' para sair da vizualização (-v).
- Lista os metadados dos arquivos da biblioteca (-l). 

## Como rodar
```bash
git clone https://github.com/PaolaCeconello/GBV-Prog-2.git
cd GBV-Prog-2
make
./gbv <opção> <biblioteca> [documentos...]
```


