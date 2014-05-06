<?php

/**
 * @file
 * Admin functions for Commerce FedEx.
 */

/**
 * Builds the admin settings form for configuring FedEx.
 *
 * @return array
 *   Drupal form for FedEx settings.
 */
function commerce_fedex_settings_form() {
  $form = array();
  $form['auth'] = array(
    '#title' => t('FedEx web authentication'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
  );
  $form['auth']['commerce_fedex_key'] = array(
    '#type' => 'textfield',
    '#title' => t('FedEx Key'),
    '#default_value' => variable_get('commerce_fedex_key'),
  );
  $form['auth']['commerce_fedex_password'] = array(
    '#type' => 'password',
    '#title' => t('FedEx Password'),
    '#description' => variable_get('commerce_fedex_password') ? t('FedEx password has been set. Leave blank unless you want to change the saved password.') : NULL,
  );
  $form['auth']['commerce_fedex_account_number'] = array(
    '#type' => 'textfield',
    '#title' => t('FedEx Account Number'),
    '#default_value' => variable_get('commerce_fedex_account_number'),
  );
  $form['auth']['commerce_fedex_meter_number'] = array(
    '#type' => 'textfield',
    '#title' => t('FedEx Meter Number'),
    '#default_value' => variable_get('commerce_fedex_meter_number'),
  );
  $form['auth']['commerce_fedex_request_mode'] = array(
    '#type' => 'radios',
    '#title' => t('FedEx Request Mode'),
    '#description' => t('If switching to production mode, make sure that you have aquired production credentials from FedEx. These will be different than you testing credentials.'),
    '#options' => array(
      'testing' => t('Testing'),
      'production' => t('Production'),
    ),
    '#default_value' => variable_get('commerce_fedex_request_mode', 'testing'),
  );
  $form['origin'] = array(
    '#type' => 'fieldset',
    '#title' => t('Ship From Address'),
    '#collapsible' => TRUE,
  );
  $form['origin']['commerce_fedex_company_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Company Name'),
    '#default_value' => variable_get('commerce_fedex_company_name'),
  );
  $form['origin']['commerce_fedex_address_line_1'] = array(
    '#type' => 'textfield',
    '#title' => t('Address'),
    '#required' => TRUE,
    '#default_value' => variable_get('commerce_fedex_address_line_1'),
  );
  $form['origin']['commerce_fedex_address_line_2'] = array(
    '#type' => 'textfield',
    '#title' => t('Address (Additional)'),
    '#default_value' => variable_get('commerce_fedex_address_line_2'),
  );
  $form['origin']['commerce_fedex_city'] = array(
    '#type' => 'textfield',
    '#title' => t('City'),
    '#required' => TRUE,
    '#default_value' => variable_get('commerce_fedex_city'),
  );
  $form['origin']['commerce_fedex_state'] = array(
    '#type' => 'textfield',
    '#title' => t('State or Province'),
    '#description' => t('If shipping from USA or Canada, enter the 2 character abbreviation for the shipping State or Province.'),
    '#default_value' => variable_get('commerce_fedex_state'),
    '#size' => 2,
  );
  $form['origin']['commerce_fedex_postal_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Postal Code'),
    '#description' => t('Enter your postal code if available.'),
    '#size' => 12,
    '#default_value' => variable_get('commerce_fedex_postal_code'),
  );
  $form['origin']['commerce_fedex_country_code'] = array(
    '#type' => 'select',
    '#title' => t('Country'),
    '#default_value' => variable_get('commerce_fedex_country_code'),
    '#options' => array('' => t('Select Country')) + commerce_fedex_serviced_countries(),
    '#required' => TRUE,
  );
  $form['services'] = array(
    '#type' => 'fieldset',
    '#title' => t('Enabled FedEx Shipping Services'),
    '#collapsible' => TRUE,
  );
  $form['services']['commerce_fedex_services'] = array(
    '#type' => 'checkboxes',
    '#options' => commerce_fedex_shipping_service_types(),
    '#default_value' => variable_get('commerce_fedex_services', array()),
  );
  $form['packaging'] = array(
    '#type' => 'fieldset',
    '#title' => t('FedEx Packaging'),
    '#collapsible' => TRUE,
  );
  $form['packaging']['commerce_fedex_default_package_type'] = array(
    '#type' => 'select',
    '#title' => t('Default package type'),
    '#options' => commerce_fedex_package_types(),
    '#default_value' => variable_get('commerce_fedex_default_package_type', 'YOUR_PACKAGING'),
  );
  $form['packaging']['default_package_size'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default package size (inches)'),
    '#collapsible' => FALSE,
    '#description' => 'FedEx requires a package size when determining estimates.',
  );
  $form['packaging']['default_package_size']['commerce_fedex_default_package_size_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Length'),
    '#size' => 5,
    '#required' => TRUE,
    '#default_value' => variable_get('commerce_fedex_default_package_size_length'),
  );
  $form['packaging']['default_package_size']['commerce_fedex_default_package_size_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#size' => 5,
    '#required' => TRUE,
    '#default_value' => variable_get('commerce_fedex_default_package_size_width'),
  );
  $form['packaging']['default_package_size']['commerce_fedex_default_package_size_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Height'),
    '#size' => 5,
    '#required' => TRUE,
    '#default_value' => variable_get('commerce_fedex_default_package_size_height'),
  );
  $form['packaging']['default_package_size']['details'] = array(
    '#markup' => 'The package size is used to determine the number of packages
     necessary to create a FedEx shipping cost estimate. <strong>If products do
     not have physical dimensions and weights associated with them, the
     estimates will not be accurate.</strong> The logic implemented works as:
     <ul>
     <li>Assume each order requires at least one package.</li>
     <li>Use the combined volume of all products in an order to determine the
     number of packages.</li>
     </ul>
     This is a simple calculation that can get close to actual shipping costs.
     More complex logic involving multiple package sizes, weights, and void
     space can be implemented via custom modules.',
  );
  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Other FedEx Options'),
    '#collapsible' => TRUE,
  );
  $form['options']['commerce_fedex_rate_service_type'] = array(
    '#type' => 'select',
    '#title' => t('Pricing options'),
    '#description' => t('Select the pricing option to use when requesting a rate quote. Note that discounted rates are only available when sending production requests.'),
    '#options' => array(
      'list' => t('Standard pricing (LIST)'),
      'account' => t('This FedEx account\'s discounted pricing (ACCOUNT)'),
    ),
    '#default_value' => variable_get('commerce_fedex_rate_service_type', 'list'),
  );
  $form['options']['commerce_fedex_shipto_residential'] = array(
    '#type' => 'select',
    '#title' => t('Ship to destination type'),
    '#description' => t('Leave this set as residential unless you only ship to commercial addresses.'),
    '#options' => array(
      'residential' => t('Residential'),
      'commercial' => t('Commercial')
    ),
    '#default_value' => variable_get('commerce_fedex_shipto_residential', 'residential'),
  );
  $form['options']['commerce_fedex_dropoff'] = array(
    '#type' => 'select',
    '#title' => t('Default dropoff/pickup location for your FedEx shipments'),
    '#options' => commerce_fedex_dropoff_types(),
    '#default_value' => variable_get('commerce_fedex_dropoff', 'REGULAR_PICKUP'),
  );
  $form['options']['commerce_fedex_insurance'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include insurance value of shippable line items in FedEx rate requests'),
    '#default_value' => variable_get('commerce_fedex_insurance'),
  );
  $form['options']['commerce_fedex_show_logo'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display FedEx Logo next to FedEx services.'),
    '#default_value' => variable_get('commerce_fedex_show_logo', 0),
  );
  $form['options']['commerce_fedex_log'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Log the following messages for debugging'),
    '#options' => array(
      'request' => t('API request messages'),
      'response' => t('API response messages'),
    ),
    '#default_value' => variable_get('commerce_fedex_log', array()),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  return $form;
}

/**
 * Validate handler for FedEx admin settings form.
 *
 * @param array $form
 *   The FedEx admin settings form.
 * @param array $form_state
 *   The FedEx admin settings form state.
 */
function commerce_fedex_settings_form_validate($form, $form_state) {
  // Define numeric fields that should be validated.
  $numeric_fields = array(
    'commerce_fedex_default_package_size_length' => t('length'),
    'commerce_fedex_default_package_size_width' => t('height'),
    'commerce_fedex_default_package_size_height' => t('width'),
  );

  // Validate any numeric fields within the submitted values.
  foreach ($numeric_fields as $key => $numeric_field) {
    if (!is_numeric($form_state['values'][$key])) {
      form_set_error($numeric_field, t('Values for @field can only be set to positive numeric values.', array('@field' => $numeric_field)));
    }
  }

  // Validate FedEx settings that are required from USA and Canada only.
  if (in_array($form_state['values']['commerce_fedex_country_code'], array('US', 'CA'))) {

    // Make sure state is provided.
    if (empty($form_state['values']['commerce_fedex_state'])) {
      form_set_error('commerce_fedex_state', t('State or Province is required when shipping from USA or Canada.'));
    }
    else {

      // Make sure state is 2 characters.
      if (!preg_match('/^[a-z][a-z]$/i', $form_state['values']['commerce_fedex_state'])) {
        form_set_error('commerce_fedex_state', t('Please enter the 2 character abbreviation for your State or Province'));
      }
    }

    // Make sure postal code is provided.
    if (empty($form_state['values']['commerce_fedex_postal_code'])) {
      form_set_error('commerce_fedex_postal_code', t('Postal code is required when shipping from USA or Canada.'));
    }
  }
}

/**
 * Submit handler for FedEx admin settings form.
 *
 * @param array $form
 *   The FedEx admin settings form.
 * @param array $form_state
 *   The FedEx admin settings form state.
 */
function commerce_fedex_settings_form_submit($form, $form_state) {
  // Check the FedEx service variable before the form is saved to check if this
  // value has been changed in this form submit.
  $services = variable_get('commerce_fedex_services', NULL);

  // Make sure State/Province value is saved as uppercase.
  if (!empty($form_state['values']['commerce_fedex_state'])) {
    $form_state['values']['commerce_fedex_state'] = drupal_strtoupper($form_state['values']['commerce_fedex_state']);
  }

  // Loop through each of the settings fields and save their values.
  foreach (commerce_fedex_settings_fields() as $key) {
    if (!empty($form_state['values'][$key])) {
      variable_set($key, $form_state['values'][$key]);
    }
    else {
      // If the value is not set then delete the variable instead of saving
      // a NULL value (unless it's the FedEx password which is only set when
      // it is updated).
      if ($key != 'commerce_fedex_password') {
        variable_del($key);
      }
    }
  }

  // If the selected shipping services have changed with this form submit then
  // clear the shipping services, rules, and menu caches.
  if ($services !== $form_state['values']['commerce_fedex_services']) {
    commerce_shipping_services_reset();
    entity_defaults_rebuild();
    rules_clear_cache(TRUE);
    menu_rebuild();
  }

  drupal_set_message(t('The FedEx configuration options have been saved.'));
}
