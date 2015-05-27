<?php

/**
 * @file
 * Rules implementation for ec_product module.
 */

/**
 * Implements hook_rules_event_info().
 */
function ec_product_rules_event_info() {
  return array(
    'transaction_product_save' => array(
      'group' => t('product'),
      'label' => t('Transaction Product Save'),
      'module' => 'ec_product',
      'variables' => array(
        'txn' => array(
          'type' => 'ec_transaction',
          'label' => t('Altered Transaction'),
        ),
        'original' => array(
          'type' => 'ec_transaction',
          'label' => t('Original Transaction'),
        ),
        'item' => array(
          'type' => 'node',
          'label' => t('Transaction Item'),
        ),
      ),
    ),
  );
}

/**
 * Implements hook_rules_condition_info().
 */
function ec_product_rules_condition_info() {
  return array(
    'ec_product_feature_exists' => array(
      'group' => t('product'),
      'label' => t('Feature exists'),
      'parameter' => array(
        'node' => array(
          'type' => 'node',
          'label' => t('Product'),
        ),
        'feature' => array(
          'type' => 'list<text>',
          'label' => t('Feature'),
          'options list' => 'ec_product_condition_feature_list',
          'restriction' => 'input',
        ),
      ),
      'module' => 'ec_store',
    ),
  );
}

function ec_product_condition_feature_list() {
  return ec_product_feature_get('names');
}
