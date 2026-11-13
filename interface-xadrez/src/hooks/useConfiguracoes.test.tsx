import { renderHook, act } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { useConfiguracoes } from './useConfiguracoes'
import { ConfiguracoesProvider } from '../contexts/ConfiguracaoContext'
import { PADRAO } from '../contexts/ConfiguracoesContext'

const renderizar = () =>
  renderHook(() => useConfiguracoes(), { wrapper: ConfiguracoesProvider })

describe('useConfiguracoes', () => {
  it('lança erro fora do Provider', () => {
    vi.spyOn(console, 'error').mockImplementation(() => {})

    expect(() => renderHook(() => useConfiguracoes())).toThrow(/ConfiguracoesProvider/)

    vi.restoreAllMocks()
  })

  it('começa com os valores padrão', () => {
    const { result } = renderizar()
    expect(result.current.config).toEqual(PADRAO)
  })

  it('atualizar altera só os campos informados', () => {
    const { result } = renderizar()

    act(() => result.current.atualizar({ volume: 0.9 }))

    expect(result.current.config).toEqual({ ...PADRAO, volume: 0.9 })
  })

  it('várias atualizações seguidas se acumulam', () => {
    const { result } = renderizar()

    act(() => {
      result.current.atualizar({ volume: 0.9 })
      result.current.atualizar({ somLigado: false })
    })

    expect(result.current.config.volume).toBe(0.9)
    expect(result.current.config.somLigado).toBe(false)
  })

  it('salva no localStorage ao atualizar', () => {
    const { result } = renderizar()

    act(() => result.current.atualizar({ escala: 0.7 }))

    expect(JSON.parse(localStorage.getItem('configuracoes')!).escala).toBe(0.7)
  })

  it('restaura o que foi salvo ao montar de novo', () => {
    const primeiro = renderizar()
    act(() => primeiro.result.current.atualizar({ volume: 0.9 }))
    primeiro.unmount()

    const segundo = renderizar()

    expect(segundo.result.current.config.volume).toBe(0.9)
  })

  it('completa com o padrão quando o salvo está incompleto', () => {
    localStorage.setItem('configuracoes', JSON.stringify({ volume: 0.9 }))

    const { result } = renderizar()

    expect(result.current.config).toEqual({ ...PADRAO, volume: 0.9 })
  })

  it('usa o padrão quando o salvo é JSON inválido', () => {
    localStorage.setItem('configuracoes', '{quebrado')

    const { result } = renderizar()

    expect(result.current.config).toEqual(PADRAO)
  })
})