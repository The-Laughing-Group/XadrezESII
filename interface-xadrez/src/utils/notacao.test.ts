import { describe, it, expect } from 'vitest'
import { posicaoToUci, uciToPosicao, desambiguacao } from './notacao'

type Tab = Parameters<typeof desambiguacao>[0]

// Monta um tabuleiro vazio e coloca as peças pelas casas, ex.: { g1: 'N', g5: 'N' }
function montar(pecas: Record<string, string>): Tab {
  const grade = Array.from({ length: 8 }, () => Array<string>(8).fill('.'))

  for (const [casa, peca] of Object.entries(pecas)) {
    const { row, col } = uciToPosicao('xx' + casa)
    grade[row][col] = peca
  }

  return grade.map((linha) => linha.join('')) as unknown as Tab
}

describe('posicaoToUci', () => {
  it.each([
    [0, 0, 'a8'],
    [0, 7, 'h8'],
    [7, 0, 'a1'],
    [7, 7, 'h1'],
    [6, 4, 'e2'],
    [4, 4, 'e4'],
    [5, 5, 'f3'],
  ])('converte (row %i, col %i) em %s', (row, col, esperado) => {
    expect(posicaoToUci(row, col)).toBe(esperado)
  })
})

describe('uciToPosicao', () => {
  it.each([
    ['e2e4', { row: 4, col: 4 }],
    ['g1f3', { row: 5, col: 5 }],
    ['a1h8', { row: 0, col: 7 }],
    ['h8a1', { row: 7, col: 0 }],
  ])('usa a casa de destino de %s', (uci, esperado) => {
    expect(uciToPosicao(uci)).toEqual(esperado)
  })

  it('ignora a letra de promoção', () => {
    expect(uciToPosicao('a7a8q')).toEqual({ row: 0, col: 0 })
    expect(uciToPosicao('e2e1n')).toEqual({ row: 7, col: 4 })
  })

  it('é o inverso de posicaoToUci para todas as 64 casas', () => {
    for (let row = 0; row < 8; row++) {
      for (let col = 0; col < 8; col++) {
        expect(uciToPosicao('xx' + posicaoToUci(row, col))).toEqual({ row, col })
      }
    }
  })
})

describe('desambiguacao', () => {
  describe('sem ambiguidade', () => {
    it('retorna vazio para peão', () => {
      const tab = montar({ e2: 'P', d2: 'P' })
      expect(desambiguacao(tab, 'e2e4')).toBe('')
    })

    it('retorna vazio para rei', () => {
      const tab = montar({ e1: 'K' })
      expect(desambiguacao(tab, 'e1e2')).toBe('')
    })

    it('retorna vazio quando é a única peça do tipo', () => {
      const tab = montar({ g1: 'N' })
      expect(desambiguacao(tab, 'g1f3')).toBe('')
    })

    it('retorna vazio quando a outra peça do mesmo tipo não alcança o destino', () => {
      const tab = montar({ g1: 'N', a1: 'N' })
      expect(desambiguacao(tab, 'g1f3')).toBe('')
    })

    it('não trata peça de cor oposta como rival', () => {
      const tab = montar({ g1: 'N', g5: 'n' })
      expect(desambiguacao(tab, 'g1f3')).toBe('')
    })

    it('não trata outro tipo de peça como rival', () => {
      const tab = montar({ a1: 'R', d4: 'Q' })
      expect(desambiguacao(tab, 'a1d1')).toBe('')
    })
  })

  describe('cavalos', () => {
    it('usa a coluna quando os rivais estão em colunas diferentes', () => {
      const tab = montar({ b1: 'N', f1: 'N' })
      expect(desambiguacao(tab, 'b1d2')).toBe('b')
      expect(desambiguacao(tab, 'f1d2')).toBe('f')
    })

    it('usa a linha quando os rivais estão na mesma coluna', () => {
      const tab = montar({ g1: 'N', g5: 'N' })
      expect(desambiguacao(tab, 'g1f3')).toBe('1')
      expect(desambiguacao(tab, 'g5f3')).toBe('5')
    })

    it('funciona para peças pretas', () => {
      const tab = montar({ b8: 'n', f8: 'n' })
      expect(desambiguacao(tab, 'b8d7')).toBe('b')
    })

    it('cavalo pula peças no caminho', () => {
      const tab = montar({ b1: 'N', f1: 'N', c1: 'B', d1: 'Q', e1: 'K', d2: '.' })
      expect(desambiguacao(tab, 'b1d2')).toBe('b')
    })
  })

  describe('torres', () => {
    it('usa a coluna quando as torres estão na mesma linha', () => {
      const tab = montar({ a1: 'R', h1: 'R' })
      expect(desambiguacao(tab, 'a1d1')).toBe('a')
      expect(desambiguacao(tab, 'h1d1')).toBe('h')
    })

    it('usa a linha quando as torres estão na mesma coluna', () => {
      const tab = montar({ a1: 'R', a7: 'R' })
      expect(desambiguacao(tab, 'a1a4')).toBe('1')
      expect(desambiguacao(tab, 'a7a4')).toBe('7')
    })

    it('ignora a torre bloqueada por outra peça', () => {
      const tab = montar({ a1: 'R', h1: 'R', f1: 'B' })
      expect(desambiguacao(tab, 'a1d1')).toBe('')
    })

    it('ignora a torre que não está em linha reta com o destino', () => {
      const tab = montar({ a1: 'R', h8: 'R' })
      expect(desambiguacao(tab, 'a1d1')).toBe('')
    })
  })

  describe('bispos', () => {
    it('usa a coluna quando os bispos estão em colunas diferentes', () => {
      const tab = montar({ c1: 'B', a3: 'B' })
      expect(desambiguacao(tab, 'c1b2')).toBe('c')
      expect(desambiguacao(tab, 'a3b2')).toBe('a')
    })

    it('ignora o bispo bloqueado no caminho diagonal', () => {
      const tab = montar({ a1: 'B', e5: 'B', d4: 'p' })
      expect(desambiguacao(tab, 'a1c3')).toBe('')
    })

    it('ignora o bispo que não está na diagonal do destino', () => {
      const tab = montar({ c1: 'B', c5: 'B' })
      expect(desambiguacao(tab, 'c1b2')).toBe('')
    })
  })

  describe('rainhas', () => {
    it('usa coluna e linha quando há rivais na mesma coluna e na mesma linha', () => {
      // a1, a3 (mesma coluna) e c1 (mesma linha) alcançam c3
      const tab = montar({ a1: 'Q', a3: 'Q', c1: 'Q' })
      expect(desambiguacao(tab, 'a1c3')).toBe('a1')
    })

    it('usa só a linha quando o rival está na mesma coluna e nenhum na mesma linha', () => {
      const tab = montar({ a1: 'Q', a3: 'Q' })
      expect(desambiguacao(tab, 'a1c3')).toBe('1')
    })

    it('usa só a coluna quando o rival está na mesma linha e nenhum na mesma coluna', () => {
      const tab = montar({ a1: 'Q', c1: 'Q' })
      expect(desambiguacao(tab, 'a1c3')).toBe('a')
    })

    it('aceita rival que alcança o destino na diagonal ou em linha reta', () => {
      const tab = montar({ a1: 'Q', e5: 'Q' })
      expect(desambiguacao(tab, 'a1c3')).toBe('a')
    })
  })
})