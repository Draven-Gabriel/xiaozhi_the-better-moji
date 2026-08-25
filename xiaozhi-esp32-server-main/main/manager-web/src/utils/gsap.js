import { gsap } from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';

gsap.registerPlugin(ScrollTrigger);

export { gsap, ScrollTrigger };

export function pageEnterAnimation(el) {
  if (!el) return null;
  return gsap.fromTo(
    el,
    { autoAlpha: 0, y: 24 },
    { autoAlpha: 1, y: 0, duration: 0.6, ease: 'power3.out' }
  );
}

export function staggerEnter(targets, options = {}) {
  if (!targets) return null;
  const { stagger = 0.08, ...rest } = options;
  return gsap.fromTo(
    targets,
    { autoAlpha: 0, y: 24 },
    { autoAlpha: 1, y: 0, duration: 0.6, ease: 'power3.out', stagger, ...rest }
  );
}

export default gsap;
