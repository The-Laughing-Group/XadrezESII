import '@testing-library/jest-dom/vitest'
import { afterEach } from 'vitest'

afterEach(() => {
  localStorage.clear()
  document.documentElement.removeAttribute('style')
  document.documentElement.className = ''
})