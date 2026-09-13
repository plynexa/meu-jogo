"""Aplica correcoes conhecidas do pack NTO Hard usado como base do Meu Jogo.

Uso:
    python ferramentas/aplicar_correcoes_dados.py <pasta-data>

Exemplo:
    python ferramentas/aplicar_correcoes_dados.py Servidor/data
"""
from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("Servidor/data")

correcoes = [
    (
        root / "spells/scripts/personagem/novos/konan/8.lua",
        "addEvent(doSendMagicEffect, 350, {x = pos.x+1, y = pos.y+, z = pos.z}, 101)",
        "addEvent(doSendMagicEffect, 350, {x = pos.x+1, y = pos.y, z = pos.z}, 101)",
        "Corrige erro de sintaxe na magia 8 de Konan",
    ),
]

for arquivo, antigo, novo, descricao in correcoes:
    if not arquivo.exists():
        raise SystemExit(f"Arquivo nao encontrado: {arquivo}")
    texto = arquivo.read_text(encoding="utf-8")
    if antigo in texto:
        arquivo.write_text(texto.replace(antigo, novo, 1), encoding="utf-8")
        print(f"OK: {descricao}")
    elif novo in texto:
        print(f"JA CORRIGIDO: {descricao}")
    else:
        raise SystemExit(f"Padrao esperado nao encontrado em: {arquivo}")
