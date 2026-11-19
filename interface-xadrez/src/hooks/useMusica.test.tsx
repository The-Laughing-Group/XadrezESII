import { renderHook, act } from '@testing-library/react'
import { afterEach, beforeEach, describe, it, expect, vi, type MockInstance } from 'vitest'
import { useMusica } from './useMusica'
import { useConfiguracoes } from './useConfiguracoes'
import { ConfiguracoesProvider } from '../contexts/ConfiguracaoContext'
import { PADRAO } from '../contexts/ConfiguracoesContext'
import musica from '../assets/sons/musica.m4a'

const GESTOS = ['pointerdown', 'pointerup', 'keydown'] as const

const play = vi.fn<() => Promise<void>>()
const pause = vi.fn()
let instancias: AudioFalso[] = []

class AudioFalso {
  src: string
  loop = false
  volume = 1
  play = play
  pause = pause
  constructor(src: string) {
    this.src = src
    instancias.push(this)
  }
}

let addSpy: MockInstance
let removeSpy: MockInstance

function renderizar() {
  return renderHook(
    () => {
      useMusica()
      return useConfiguracoes()
    },
    { wrapper: ConfiguracoesProvider }
  )
}

const esperar = () => act(async () => {})

const gesto = (tipo: (typeof GESTOS)[number]) => window.dispatchEvent(new Event(tipo))

const bloqueiaAutoplay = () =>
  play.mockRejectedValueOnce(new DOMException('bloqueado', 'NotAllowedError'))

describe('useMusica', () => {
  beforeEach(() => {
    instancias = []
    play.mockReset().mockResolvedValue(undefined)
    pause.mockReset()
    vi.stubGlobal('Audio', AudioFalso)
    addSpy = vi.spyOn(window, 'addEventListener')
    removeSpy = vi.spyOn(window, 'removeEventListener')
  })

  afterEach(() => {
    vi.unstubAllGlobals()
    vi.restoreAllMocks()
  })

  describe('criação do áudio', () => {
    it('cria um único áudio com a música, em loop e com o volume da configuração', () => {
      renderizar()

      expect(instancias).toHaveLength(1)
      expect(instancias[0].src).toBe(musica)
      expect(instancias[0].loop).toBe(true)
      expect(instancias[0].volume).toBe(PADRAO.volumeMusica)
    })

    it('não recria o áudio nem toca de novo em um re-render sem mudança de config', () => {
      const { rerender } = renderizar()

      rerender()

      expect(instancias).toHaveLength(1)
      expect(play).toHaveBeenCalledOnce()
    })

    it('pausa a música ao desmontar', () => {
      const { unmount } = renderizar()

      unmount()

      expect(pause).toHaveBeenCalledOnce()
    })
  })

  describe('música ligada e desligada', () => {
    it('toca ao montar com a música ligada', () => {
      renderizar()

      expect(play).toHaveBeenCalledOnce()
      expect(pause).not.toHaveBeenCalled()
    })

    it('não toca ao montar com a música desligada, e garante que está pausada', () => {
      localStorage.setItem('configuracoes', JSON.stringify({ musicaLigada: false }))

      renderizar()

      expect(play).not.toHaveBeenCalled()
      expect(pause).toHaveBeenCalledOnce()
    })

    it('pausa ao desligar a música', () => {
      const { result } = renderizar()

      act(() => result.current.atualizar({ musicaLigada: false }))

      expect(pause).toHaveBeenCalledOnce()
    })

    it('volta a tocar ao religar a música', () => {
      const { result } = renderizar()

      act(() => result.current.atualizar({ musicaLigada: false }))
      act(() => result.current.atualizar({ musicaLigada: true }))

      expect(play).toHaveBeenCalledTimes(2)
    })
  })

  describe('volume', () => {
    it('atualiza o volume sem recriar o áudio', () => {
      const { result } = renderizar()

      act(() => result.current.atualizar({ volumeMusica: 0.8 }))

      expect(instancias).toHaveLength(1)
      expect(instancias[0].volume).toBe(0.8)
    })

    it('atualiza o volume mesmo com a música desligada, sem tocar', () => {
      localStorage.setItem('configuracoes', JSON.stringify({ musicaLigada: false }))
      const { result } = renderizar()

      act(() => result.current.atualizar({ volumeMusica: 0.7 }))

      expect(instancias[0].volume).toBe(0.7)
      expect(play).not.toHaveBeenCalled()
    })

    it('mudar o volume dos efeitos não mexe na música', () => {
      const { result } = renderizar()

      act(() => result.current.atualizar({ volume: 0.9 }))

      expect(play).toHaveBeenCalledOnce()
      expect(instancias[0].volume).toBe(PADRAO.volumeMusica)
    })
  })

  describe('autoplay bloqueado pelo navegador', () => {
    it('não registra gestos quando a reprodução funciona', async () => {
      renderizar()
      await esperar()

      for (const g of GESTOS) {
        expect(addSpy).not.toHaveBeenCalledWith(g, expect.any(Function), { once: true })
      }
    })

    it('registra os três gestos (uma vez cada) quando a reprodução é bloqueada', async () => {
      bloqueiaAutoplay()
      renderizar()
      await esperar()

      for (const g of GESTOS) {
        expect(addSpy).toHaveBeenCalledWith(g, expect.any(Function), { once: true })
      }
    })

    it.each(GESTOS)('tenta tocar de novo no primeiro gesto "%s"', async (tipo) => {
      bloqueiaAutoplay()
      renderizar()
      await esperar()
      expect(play).toHaveBeenCalledTimes(1)

      gesto(tipo)

      expect(play).toHaveBeenCalledTimes(2)
    })

    it('o mesmo gesto só dispara uma tentativa (once)', async () => {
      bloqueiaAutoplay()
      renderizar()
      await esperar()

      gesto('pointerdown')
      gesto('pointerdown')

      expect(play).toHaveBeenCalledTimes(2)
    })

    it('ignora a falha quando o navegador ainda bloqueia depois do gesto', async () => {
      play.mockRejectedValue(new DOMException('bloqueado', 'NotAllowedError'))
      renderizar()
      await esperar()

      expect(() => gesto('keydown')).not.toThrow()
      await esperar()
    })

    it('desligar a música remove os gestos e eles não tocam mais', async () => {
      bloqueiaAutoplay()
      const { result } = renderizar()
      await esperar()

      act(() => result.current.atualizar({ musicaLigada: false }))
      gesto('pointerdown')

      for (const g of GESTOS) {
        expect(removeSpy).toHaveBeenCalledWith(g, expect.any(Function))
      }
      expect(play).toHaveBeenCalledTimes(1)
    })

    it('desmontar remove os gestos e eles não tocam mais', async () => {
      bloqueiaAutoplay()
      const { unmount } = renderizar()
      await esperar()

      unmount()
      gesto('keydown')

      for (const g of GESTOS) {
        expect(removeSpy).toHaveBeenCalledWith(g, expect.any(Function))
      }
      expect(play).toHaveBeenCalledTimes(1)
    })

    it('não registra gestos se o efeito já foi limpo quando a reprodução falha', async () => {
      let rejeitar!: (erro: unknown) => void
      play.mockReturnValue(new Promise<void>((_, rej) => { rejeitar = rej }))
      const { unmount } = renderizar()

      unmount()
      rejeitar(new DOMException('bloqueado', 'NotAllowedError'))
      await esperar()

      for (const g of GESTOS) {
        expect(addSpy).not.toHaveBeenCalledWith(g, expect.any(Function), { once: true })
      }
    })
  })
})