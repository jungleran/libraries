<?php

/**
 * @file
 * Contains \Drupal\libraries\ExternalLibrary\Asset\LocalRemoteAssetTrait.
 */

namespace Drupal\libraries\ExternalLibrary\Asset;

use Drupal\Component\Plugin\Factory\FactoryInterface;

/**
 * A trait for asset libraries that serve local and remote files.
 *
 * If the library files are available locally, they are served locally.
 * Otherwise, the remote files are served, assuming a remote URL is specified.
 *
 * This trait should only be used in classes implementing AssetLibraryInterface,
 * LocalLibraryInterface and RemoteLibraryInterface.
 *
 * @see \Drupal\libraries\ExternalLibrary\Asset\SingleAssetLibraryTrait
 * @see \Drupal\libraries\ExternalLibrary\Asset\AssetLibraryInterface
 * @see \Drupal\libraries\ExternalLibrary\Local\LocalLibraryInterface
 * @see \Drupal\libraries\ExternalLibrary\Remote\RemoteLibraryInterface
 */
trait LocalRemoteAssetTrait {

  /**
   * An array containing the CSS assets of the library.
   *
   * @var array
   */
  protected $cssAssets;

  /**
   * An array containing the JavaScript assets of the library.
   *
   * @var array
   */
  protected $jsAssets;

  /**
   * Gets the locator of this library using the locator factory.
   *
   * Because determining the installation status and library path of a library
   * is not specific to any library or even any library type, this logic is
   * offloaded to separate locator objects.
   *
   * @param \Drupal\Component\Plugin\Factory\FactoryInterface $locator_factory
   *
   * @return \Drupal\libraries\ExternalLibrary\Local\LocatorInterface
   *
   * @see \Drupal\libraries\ExternalLibrary\Local\LocalLibraryInterface::getLocator()
   */
  public function getLocator(FactoryInterface $locator_factory) {
    return $locator_factory->createInstance('stream', ['scheme' => 'asset']);
  }

  /**
   * Checks whether this library can be attached.
   *
   * @return bool
   *   TRUE if the library can be attached; FALSE otherwise.
   *
   * @see \Drupal\libraries\ExternalLibrary\Asset\SingleAssetLibraryTrait::canBeAttached()
   */
  protected function canBeAttached() {
    /** @var \Drupal\libraries\ExternalLibrary\Local\LocalLibraryInterface|\Drupal\libraries\ExternalLibrary\Remote\RemoteLibraryInterface $this */
    return ($this->isInstalled() || $this->hasRemoteUrl());
  }

  /**
   * Gets the CSS assets attached to this library.
   *
   * @return array
   *   An array of CSS assets of the library following the core library CSS
   *   structure. The keys of the array must be among the SMACSS categories
   *   'base', 'layout, 'component', 'state', and 'theme'. The value of each
   *   category is in turn an array where the keys are the file paths of the CSS
   *   files and values are CSS options.
   *
   * @see https://smacss.com/
   *
   * @see \Drupal\libraries\ExternalLibrary\Asset\SingleAssetLibraryTrait::getCssAssets()
   */
  protected function getCssAssets() {
    // @todo Process the paths.
    return $this->cssAssets;
  }

  /**
   * Gets the JavaScript assets attached to this library.
   *
   * @return array
   *   An array of JavaScript assets of the library. The keys of the array are
   *   the file paths of the JavaScript files and the values are JavaScript
   *   options.
   *
   * @see \Drupal\libraries\ExternalLibrary\Asset\SingleAssetLibraryTrait::getJsAssets()
   */
  protected function getJsAssets() {
    // @todo Process the paths.
    return $this->jsAssets;
  }

}
