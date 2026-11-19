import { renderHook, act } from '@testing-library/react'
import { afterEach, beforeEach, describe, it, expect, vi } from 'vitest'
import { useSom } from './useSom'
import { useConfiguracoes } from './useConfiguracoes'
import { ConfiguracoesProvider } from '../contexts/ConfiguracaoContext'
import movimento from '../assets/sons/movimento.m4a'
import promocao from '../assets/sons/promocao.mp3'
import xeque from '../assets/sons/xeque.mp3'
import fimjogo from '../assets/sons/fimjogo.mp3'

const play = vi.fn<() => Promise<void>>()
let instancias: AudioFalso[] = []

class AudioFalso {
  src: string
  volume = 1
  play = play
  constructor(src: string) {
    this.src = src
    instancias.push(this)
  }
}

function renderizar(tipo?: Parameters<typeof useSom>[0]) {
  return renderHook(
    () => ({ tocar: useSom(tipo), cfg: useConfiguracoes() }),
    { wrapper: ConfiguracoesProvider }
  )
}

describe('useSom', () => {
  beforeEach(() => {
    instancias = []
    play.mockReset().mockResolvedValue(undefined)
    vi.stubGlobal('Audio', AudioFalso)
  })

  afterEach(() => {
    vi.unstubAllGlobals()
  })

  it('toca o som de movimento por padrão, com o volume da configuração', () => {
    const { result } = renderizar()

    result.current.tocar()

    expect(instancias).toHaveLength(1)
    expect(instancias[0].src).toBe(movimento)
    expect(instancias[0].volume).toBe(0.5) // PADRAO.volume
    expect(play).toHaveBeenCalledOnce()
  })

  it.each([
    ['movimento', movimento],
    ['promocao', promocao],
    ['xeque', xeque],
    ['fimjogo', fimjogo],
  ] as const)('o tipo "%s" usa o arquivo correto', (tipo, arquivo) => {
    const { result } = renderizar(tipo)

    result.current.tocar()

    expect(instancias[0].src).toBe(arquivo)
  })

  it('não cria nem toca áudio com os efeitos sonoros desligados', () => {
    const { result } = renderizar()

    act(() => result.current.cfg.atualizar({ somLigado: false }))
    result.current.tocar()

    expect(instancias).toHaveLength(0)
    expect(play).not.toHaveBeenCalled()
  })

  it('usa o volume atualizado depois de mudar a configuração', () => {
    const { result } = renderizar()

    act(() => result.current.cfg.atualizar({ volume: 0.9 }))
    result.current.tocar()

    expect(instancias[0].volume).toBe(0.9)
  })

  it('volta a tocar ao religar os efeitos sonoros', () => {
    const { result } = renderizar()

    act(() => result.current.cfg.atualizar({ somLigado: false }))
    act(() => result.current.cfg.atualizar({ somLigado: true }))
    result.current.tocar()

    expect(play).toHaveBeenCalledOnce()
  })

  it('ignora o erro quando o navegador bloqueia a reprodução', async () => {
    play.mockRejectedValue(new DOMException('bloqueado', 'NotAllowedError'))
    const { result } = renderizar()

    expect(() => result.current.tocar()).not.toThrow()
    await Promise.resolve()
  })
})