<?php

/**
 * Submission callback for fieldable_panels_panes_fieldable_panels_pane_content_type_edit_form().
 */
function _panels_preview_fieldable_panels_panes_fieldable_panels_pane_content_type_edit_form_submit($form, &$form_state) {
  $entity = $form_state['entity'];

  // The live preview causes a stale version of the entity to exist in the
  // entity controller's static cache, so we much clear it on submit.
  entity_get_controller('fieldable_panels_pane')->resetCache(array($entity->fpid));
}

/**
 * Renders previews for fieldable_panels_pane content types.
 */
function panels_preview_render_fieldable_panels_pane_preview($type, $subtype, $conf, $keywords, $args, $context) {
  $entity = $conf['fieldable_panels_pane'];
  if ($entity && fieldable_panels_panes_access('view', $entity)) {
    $settings = field_bundle_settings('fieldable_panels_pane', $entity->bundle);
    $block = new stdClass();
    $block->type = $type;
    $block->subtype = $subtype;
    $block->title = '';
    if (empty($settings['extra_fields']['display']) || !empty($settings['extra_fields']['display']['title']['default']['visible'])) {
      if (!empty($entity->title)) {
        $block->title = check_plain($entity->title);
      }
    }

    // Some magic necessary to make sure fields are in the same state they
    // would be in if they were loaded from the database. Found this by looking
    // at how node generates it's previews in node_preview().
    _field_invoke_multiple('load', 'fieldable_panels_pane', array($entity->fpid => $entity));

    $view_mode = isset($conf['view_mode']) ? $conf['view_mode'] : 'full';
    $block->content = fieldable_panels_panes_view($entity, $view_mode);

    return $block;
  }
}

/**
 * Implementation of hook_form_FORM_ID_alter()
 */
function panels_preview_form_fieldable_panels_panes_fieldable_panels_pane_content_type_edit_form_alter(&$form, &$form_state, $form_id) {

  // Setup the live preview
  if (variable_get('panels_preview_live_preview', 1)) {
    panels_preview_autosubmit_configure($form);

    // Special case exclusions for tablefield specific elements.
    if ($form_state['entity']->bundle == 'table') {
      $form['field_basic_table_table'][LANGUAGE_NONE][0]['tablefield']['rebuild']['count_cols']['#attributes']['class'] = array('ctools-auto-submit-exclude');
      $form['field_basic_table_table'][LANGUAGE_NONE][0]['tablefield']['rebuild']['count_rows']['#attributes']['class'] = array('ctools-auto-submit-exclude');
    }
  }

  // Add custom validation function for the reusable title
  if (!empty($form_state['triggering_element']) && $form_state['triggering_element']['#value'] != 'Update Preview') {
    $form['reusable']['admin_title']['#element_validate'][] = 'panels_preview_reusable_entity_validate';
  }

  // Customize the view mode setting
  if (!empty($form['view_mode'])) {
    $form['view_mode']['#options'] = panels_preview_view_mode_options('fieldable_panels_pane');
    if (count($form['view_mode']['#options']) < 2) {
      $form['view_mode']['#type'] = 'value';
      $form['view_mode']['#value'] = key($form['view_mode']['#options']);
    }
    $form['widget_settings']['view_mode'] = $form['view_mode'];
    unset($form['view_mode']);
  }

  // Improving the general title setting
  if (!empty($form['widget_settings']['title'])) {
    $form['widget_settings']['title']['#size'] = '';
  }

  // Improving the link Options
  if (!empty($form['widget_settings']['link'])) {
    $form['widget_settings']['link']['path']['#size'] = '';
    $form['widget_settings']['link']['path']['#field_prefix'] =  $form['widget_settings']['link']['path']['#title'];
    $form['widget_settings']['link']['path']['#title'] = '';
    $form['widget_settings']['link']['path']['#description'] = t('The URL or Drupal path from which to build the link.');
    $form['widget_settings']['link']['link']['#description'] = '';
  }

  // Move the reusable option to the general settings
  if (!empty($form['reusable'])) {
    $form['reusable']['#type'] = '';
    $form['reusable']['#title'] = '';
    $form['reusable']['reusable']['#title'] = t('Make this reusable');
    $form['reusable']['admin_title']['#field_prefix'] = t('Admin Title');
    $form['reusable']['admin_title']['#title'] = '';
    $form['reusable']['admin_title']['#size'] = '';
    $form['reusable']['admin_title']['#description'] = '';
    $form['widget_settings']['reusable'] = $form['reusable'];
    unset($form['reusable']);
  }
}