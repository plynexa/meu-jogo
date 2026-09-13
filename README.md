# Meu Jogo

Projeto de RPG urbano inspirado na perspectiva e movimentação de Tibia, com sprites próprias, mobs e cenário baseado em locais reais. Nome provisório.

## Objetivo de uso

Extrair um pacote Windows, iniciar o servidor local e o cliente por atalhos e abrir os editores com os caminhos configurados. O protótipo deve funcionar localmente antes da implantação online.

## Arquitetura pretendida

- Cliente: apresentação, controles, sprites, sons e animações locais.
- Servidor: validação de movimento, combate, mobs, inventário e persistência.
- Rede: comandos e atualizações de estado; recursos gráficos distribuídos com o cliente ou atualizações.
- Plataforma inicial de edição: Windows. Suporte a Android e toque deve ser avaliado antes de consolidar o cliente definitivo.

## Estado atual — ainda não há versão jogável para download

O pacote de referência é NTO Hard - FULL PACK by RobertoAsgon.zip. Sua análise identificou protocolo 8.60, fontes C++ do servidor, binários de servidor Linux e editores Windows existentes.

Pendências obrigatórias:

- Preparar e executar a compilação Windows com dependências compatíveis.
- Preparar o esquema do banco: não foi encontrado dump SQL no pacote.
- Resolver a incompatibilidade entre o mapa e os arquivos de itens, preservando o original.
- Configurar conexão local e conta/personagem de teste.
- Testar inicialização, entrada no mapa, movimento, combate e persistência no Windows.
- Só então empacotar e disponibilizar uma versão validada.

Os atalhos preparados anteriormente não equivalem a um servidor compilado ou validado.

## Organização planejada

| Pasta | Conteúdo |
|---|---|
| servidor/ | Código e configurações de exemplo sem senhas |
| cliente/ | Código e configuração do cliente escolhido |
| artes/ | Sprites próprias e arquivos de trabalho |
| mapas/ | Mapas e configurações de spawns |
| ferramentas/ | Scripts de preparação e empacotamento |
| docs/ | Instruções e resultados de validação |

Os diretórios serão adicionados quando receberem arquivos. O ZIP de terceiros não foi publicado integralmente neste repositório.

## Distribuição

O repositório guarda o código. Downloads do cliente poderão ser publicados em Releases após validação. O processo do servidor online será executado em hospedagem separada.
