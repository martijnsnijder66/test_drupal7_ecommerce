<?php
/**
 * @file
 * Provide standard methods for charges.
 */

/**
 * Implements hook_ec_charge_filter_info().
 */
function ec_charge_ec_charge_filter_info() {
  return array(
    'or' => array(
      'name' => t('Or'),
      'description' => t('Allow filters to be or\'ed together with other filters'),
      'module' => 'ec_charge_or',
      'file' => 'ec_charge.inc',
      'core' => TRUE,
      'no form' => TRUE,
      'parent' => TRUE,
      'weight' => -1,
    ),
    'variable' => array(
      'name' => t('Variable'),
      'description' => t('Filter based upon a defined variable'),
      'module' => 'ec_charge_variable',
      'file' => 'ec_charge.inc',
      'core' => TRUE,
    ),
  );
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function ec_charge_or_filter_process($type, $settings, $object, $charges, $chg) {
  _ec_charge_or_filter_children($settings['id'], TRUE);
  $child_filters = array_filter($chg->filters, '_ec_charge_or_filter_children');

  if (empty($child_filters)) {
    return TRUE;
  }
  else {
    foreach ($child_filters as $filter) {
      $ret = ec_charge_invoke_callback('filter', $filter['component'], 'process', $type, $filter, $object, $charges, $chg);

      if ($filter['reverse']) {
        $ret = !$ret;
      }

      if ($ret) {
        return TRUE;
      }
    }
    return FALSE;
  }
}

function _ec_charge_or_filter_children($a, $init = FALSE) {
  $parent = & drupal_static(__FUNCTION__ . '_parent', 0);

  if ($init) {
    $parent = $a;
    return;
  }

  return $a['parent'] == $parent;
}

function ec_charge_variable_filter_settings() {
  return array('variable', 'op', 'value');
}

function ec_charge_variable_filter_form(&$form_state, $settings = array()) {
  //dpm($form_state);
  $chg =& $form_state['chg'];
  $variables = ec_charge_get_variables($chg, $chg->chg_type ? 'node' : 'txn', NULL);
  $descriptions = ec_charge_get_variables_description($chg, $chg->chg_type ? 'node' : 'txn', NULL, $variables);
  
  $options = array_merge(array('' => t('Select a variable')), array_unique($descriptions));
  $form['variable'] = array(
    '#type' => 'select',
    '#title' => t('Variable'),
    '#default_value' => $settings['variable'],
    '#options' => $options,
  );
  
  $form['op'] = array(
    '#type' => 'select',
    '#title' => t('Operand'),
    '#default_value' => $settings['op'],
    '#options' => array(
      '=' => t('Equals'),
      '!=' => t('Not Equal'),
      '>' => t('Greater than'),
      '<' => t('Less than'),
      '>=' => t('Greater than or equal'),
      '<=' => t('Less than or equal'),
    ),
  );
  
  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Value'),
    '#default_value' => $settings['value'],
    '#element_validate' => array('valid_number'),
  );
  
  return $form;
}

function ec_charge_variable_filter_process($type, $settings, $object, $charges = array(), $chg = NULL, $variables = array()) {
  if ($type == 'txn') {
    $variable = $variables[$settings['variable']]+0;
    
    switch ($settings['op']) {
      case '=':
        return ($variable == $settings['value']);

      case '!=':
        return ($variable != $settings['value']);
        
      case '>':
        return ($variable > $settings['value']);
      
      case '<':
        return ($variable < $settings['value']);
      
      case '>=':
        return ($variable >= $settings['value']);
      
      case '<=':
        return ($variable <= $settings['value']);
      
      default:
        return FALSE;
    }
  }
}
