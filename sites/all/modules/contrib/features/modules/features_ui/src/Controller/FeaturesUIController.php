<?php

/**
 * @file
 * Contains \Drupal\features_ui\Controller\FeaturesUIController
 */

namespace Drupal\features_ui\Controller;

use Drupal\Core\DependencyInjection\ContainerInjectionInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\JsonResponse;
use Drupal\features\FeaturesManagerInterface;
use Drupal\features\FeaturesAssignerInterface;

/**
 * Returns ajax responses for the Features UI.
 */
class FeaturesUIController implements ContainerInjectionInterface {

  /**
   * The features manager.
   *
   * @var \Drupal\features\FeaturesManagerInterface
   */
  protected $featuresManager;

  /**
   * The package assigner.
   *
   * @var array
   */
  protected $assigner;

  /**
   * Constructs a new FeaturesUIController object.
   *
   * @param \Drupal\features\FeaturesManagerInterface $features_manager
   *    The features manager.
   */
  public function __construct(FeaturesManagerInterface $features_manager, FeaturesAssignerInterface $assigner) {
    $this->featuresManager = $features_manager;
    $this->assigner = $assigner;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('features.manager'),
      $container->get('features_assigner')
    );
  }

  /**
   * Return a list of auto-detected config items for a feature
   * @param string $name
   *   short machine name of feature to process
   * @return array
   *   list of auto-detected config items, keyed by type and short name
   */
  public function detect($name) {
    $detected = array();
    $this->assigner->assignConfigPackages();
    $config_collection = $this->featuresManager->getConfigCollection();

    $items = $_POST['items'];
    if (!empty($items)) {
      $excluded = (!empty($_POST['excluded'])) ? $_POST['excluded'] : array();
      $selected = array();
      foreach ($items as $key) {
        preg_match('/^([^\[]+)(\[.+\])?\[(.+)\]\[(.+)\]$/', $key, $matches);
        if (!empty($matches[1]) && !empty($matches[4])) {
          $component = $matches[1];
          $item = $this->domDecode($matches[4]);
          if (!isset($excluded[$component][$item])) {
            $selected[] = $this->featuresManager->getFullName($component, $item);
          }
        }
      }
      $detected = !empty($selected) ? $this->getConfigDependents($selected) : array();
      $detected = array_merge($detected, $selected);
    }

    $result = [];
    foreach ($detected as $name) {
      $item = $config_collection[$name];
      $result[$item['type']][$item['name_short']] = $item['name'];
    }
    return new JsonResponse($result);
  }

  protected function getConfigDependents(array $item_names = NULL) {
    $result = [];
    $config_collection = $this->featuresManager->getConfigCollection();
    $packages = $this->featuresManager->getPackages();
    $settings = $this->featuresManager->getSettings();
    $allow_conflicts = $settings->get('conflicts');

    if (empty($item_names)) {
      $item_names = array_keys($config_collection);
    }

    foreach ($item_names as $item_name) {
      if (!empty($config_collection[$item_name]['package'])) {
        foreach ($config_collection[$item_name]['dependents'] as $dependent_item_name) {
          if (isset($config_collection[$dependent_item_name])) {
            $allow = TRUE;
            if (!$allow_conflicts && !empty($config_collection[$dependent_item_name]['package'])) {
              if (isset($packages[$config_collection[$dependent_item_name]['package']])) {
                $allow = $packages[$config_collection[$dependent_item_name]['package']]['status'] == FeaturesManagerInterface::STATUS_NO_EXPORT;
              }
            }
            if ($allow) {
              $result[] = $dependent_item_name;
            }
          }
        }
      }
    }
    return $result;
  }

  protected function domEncode($key) {
    $replacements = $this->domEncodeMap();
    return strtr($key, $replacements);
  }

  protected function domDecode($key) {
    $replacements = array_flip($this->domEncodeMap());
    return strtr($key, $replacements);
  }

  /**
   * Returns encoding map for decode and encode options.
   */
  protected function domEncodeMap() {
    return array(
      ':' => '__' . ord(':') . '__',
      '/' => '__' . ord('/') . '__',
      ',' => '__' . ord(',') . '__',
      '.' => '__' . ord('.') . '__',
      '<' => '__' . ord('<') . '__',
      '>' => '__' . ord('>') . '__',
      '%' => '__' . ord('%') . '__',
      ')' => '__' . ord(')') . '__',
      '(' => '__' . ord('(') . '__',
    );
  }

}
