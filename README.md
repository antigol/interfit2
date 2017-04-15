Interfit
========

This repository depends on other github repository via the system of submodules.
Therefore, to clone it properly you will need to type `git clone --recursive url`.

It depends only on Qt. The sound acquisition is done by the Qt Multimedia module (class QAudioInput).
Therefore, it is recommended to install Qt direcly from the [web site](https://www.qt.io/download-open-source).

This program attempts to acquire measure from laser interferometry during the deposition of a layer on a sample. And to fit the measures with a theoretical model.

The asumptions of the model are the following:
- Only one layer on top of the substrate
- The layer and the substrate are characterized by their complex refractive index (the imaginary part correspond to the absorption)
- Constant rate deposition
- Monochromatic laser

This program acquire data from two lockins via two soudcards. One lockin measure the intensity of the laser before the reflexion with the sample and the second one measure the reflexion intensity.

The fitting algorithm is [Metropolis–Hastings](https://en.wikipedia.org/wiki/Metropolis%E2%80%93Hastings_algorithm) with [Parallel tempering](https://en.wikipedia.org/wiki/Parallel_tempering).
This physical statistical algorithms can be used to fit a function with the help of the [Bayesian inference](https://en.wikipedia.org/wiki/Bayesian_inference).
- The mean square error (MSE) plays the role of the energy
- The temperatures are set to `T_k = T_0 * 3^k` to uniformize the swap probability under the hypothesis `<E_k> approx T_k`
