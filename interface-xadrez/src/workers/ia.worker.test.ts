import { afterEach, beforeEach, describe, it, expect, vi } from 'vitest'

const { carregarModulo, carregarFen, melhorLanceIA, construtor } = vi.hoisted(() => ({
  carregarModulo: vi.fn(),
  carregarFen: vi.fn(),
  melhorLanceIA: vi.fn(),
  construtor: vi.fn(),
}))

vi.mock('../hooks/useJogo', () => ({ carregarModulo }))

class JogoFalso {
  carregarFen = carregarFen
  melhorLanceIA = melhorLanceIA
  constructor() {
    construtor()
  }
}

interface PedidoIA { id: number; fen: string; profundidade: number }

const FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'

async function carregarWorker() {
  vi.resetModules()
  await import('./ia.worker')
}

function enviar(pedido: PedidoIA) {
  const handler = self.onmessage as unknown as (e: MessageEvent<PedidoIA>) => Promise<void>
  return handler(new MessageEvent('message', { data: pedido }))
}

describe('worker da IA', () => {
  let postMessage: ReturnType<typeof vi.spyOn>

  beforeEach(() => {
    carregarModulo.mockReset().mockResolvedValue({ Jogo: JogoFalso })
    carregarFen.mockReset().mockReturnValue(true)
    melhorLanceIA.mockReset().mockReturnValue('e2e4')
    construtor.mockReset()
    postMessage = vi.spyOn(self, 'postMessage').mockImplementation(() => {})
  })

  afterEach(() => {
    self.onmessage = null
    vi.restoreAllMocks()
  })

  describe('inicialização', () => {
    it('carrega o módulo WASM e cria uma única instância do jogo', async () => {
      await carregarWorker()
      await enviar({ id: 1, fen: FEN, profundidade: 3 })
      await enviar({ id: 2, fen: FEN, profundidade: 3 })

      expect(carregarModulo).toHaveBeenCalledOnce()
      expect(construtor).toHaveBeenCalledOnce()
    })

    it('registra o handler de mensagens', async () => {
      await carregarWorker()

      expect(self.onmessage).toBeTypeOf('function')
    })

    it('só responde depois que o módulo termina de carregar', async () => {
      let liberar!: (modulo: unknown) => void
      carregarModulo.mockReturnValue(new Promise((resolve) => { liberar = resolve }))
      await carregarWorker()

      const resposta = enviar({ id: 1, fen: FEN, profundidade: 3 })
      await Promise.resolve()
      expect(postMessage).not.toHaveBeenCalled()
      expect(carregarFen).not.toHaveBeenCalled()

      liberar({ Jogo: JogoFalso })
      await resposta

      expect(postMessage).toHaveBeenCalledWith({ id: 1, uci: 'e2e4' })
    })
  })

  describe('pedido de lance', () => {
    beforeEach(carregarWorker)

    it('carrega o FEN, pede o melhor lance com a profundidade e responde com o UCI', async () => {
      await enviar({ id: 7, fen: FEN, profundidade: 4 })

      expect(carregarFen).toHaveBeenCalledWith(FEN)
      expect(melhorLanceIA).toHaveBeenCalledWith(4)
      expect(postMessage).toHaveBeenCalledOnce()
      expect(postMessage).toHaveBeenCalledWith({ id: 7, uci: 'e2e4' })
    })

    it('carrega o FEN antes de calcular o lance', async () => {
      const ordem: string[] = []
      carregarFen.mockImplementation(() => { ordem.push('fen'); return true })
      melhorLanceIA.mockImplementation(() => { ordem.push('lance'); return 'e2e4' })

      await enviar({ id: 1, fen: FEN, profundidade: 2 })

      expect(ordem).toEqual(['fen', 'lance'])
    })

    it('devolve o mesmo id do pedido, para o chamador casar pedido e resposta', async () => {
      melhorLanceIA.mockReturnValueOnce('e2e4').mockReturnValueOnce('d2d4')

      await enviar({ id: 10, fen: FEN, profundidade: 2 })
      await enviar({ id: 11, fen: FEN, profundidade: 2 })

      expect(postMessage).toHaveBeenNthCalledWith(1, { id: 10, uci: 'e2e4' })
      expect(postMessage).toHaveBeenNthCalledWith(2, { id: 11, uci: 'd2d4' })
    })

    it('responde a todos os pedidos enviados em sequência rápida', async () => {
      await Promise.all([
        enviar({ id: 1, fen: FEN, profundidade: 1 }),
        enviar({ id: 2, fen: FEN, profundidade: 1 }),
        enviar({ id: 3, fen: FEN, profundidade: 1 }),
      ])

      expect(postMessage.mock.calls.map(([resposta]) => resposta.id).sort()).toEqual([1, 2, 3])
    })
  })

  describe('casos sem lance', () => {
    beforeEach(carregarWorker)

    it('responde uci null quando o FEN é inválido, sem calcular lance', async () => {
      carregarFen.mockReturnValue(false)

      await enviar({ id: 5, fen: 'lixo', profundidade: 3 })

      expect(melhorLanceIA).not.toHaveBeenCalled()
      expect(postMessage).toHaveBeenCalledWith({ id: 5, uci: null })
    })

    it('responde uci null quando a engine devolve string vazia (sem lances legais)', async () => {
      melhorLanceIA.mockReturnValue('')

      await enviar({ id: 6, fen: FEN, profundidade: 3 })

      expect(postMessage).toHaveBeenCalledWith({ id: 6, uci: null })
    })

    it('um FEN inválido não contamina o pedido seguinte', async () => {
      carregarFen.mockReturnValueOnce(false).mockReturnValueOnce(true)

      await enviar({ id: 1, fen: 'lixo', profundidade: 3 })
      await enviar({ id: 2, fen: FEN, profundidade: 3 })

      expect(postMessage).toHaveBeenNthCalledWith(1, { id: 1, uci: null })
      expect(postMessage).toHaveBeenNthCalledWith(2, { id: 2, uci: 'e2e4' })
    })
  })
})