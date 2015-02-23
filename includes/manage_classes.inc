<?php
/*
* @file
* Holds  functions to manage classes vs models for islandora_ontologies.
*/
$module_path = drupal_get_path('module', 'islandora_ontologies');
include_once($module_path.'/arc2/ARC2.php');
include_once($module_path.'/Graphite/Graphite.php');

/**
* This function reads models and display a table to assign model to class object
*
*/

function islandora_ontologies_parsed_class_view_form(array $form, array &$form_state, $islandora_object_pid){
	module_load_include('inc', 'islandora', 'includes/utilities');

	$PID = $islandora_object_pid->id;

	$model_list = islandora_ontology_model_object_members();
	
	$rows = array();
		
	foreach ($model_list as $model) {
		$mpid = $model['object']['value'];
		$excluded_model = 'fedora-system';
		$excluded_model_onto = 'ontologies';
		if (!(strpos($mpid,$excluded_model)===0) AND !(strpos($mpid,$excluded_model_onto)===0)){

			$existingassociation=0;			
			$subject= islandora_object_load($mpid);
			if (!$subject) {
				drupal_set_message("Fedora object doesn't exist.");
				drupal_not_found();
			exit;
			}
			$rels = $subject->relationships->get('info:fedora/fedora-system:def/relations-external#', 'isMemberOf', $PID);

			if (count($rels)){
					$existingassociation=1;
			}

			$rows[$mpid] = array(
				'selected' => array(
					'#type' => 'checkbox',
					'#default_value' => $existingassociation,
				),

				'modelpid' => array(
					'#type' => 'textfield',
					'#default_value' => $mpid,
					'#disabled' => TRUE,
					'#size' => 40,
				),
				'status' => array(
					'#type' => 'checkbox',
					'#default_value' => $existingassociation,
					'#disabled' => TRUE,
				),			
			);
		}
		
	}

	$form['table'] = array(
			'#tree' => TRUE,
			'#header' => array(
				array('model' => array('select-all')), t('Model'), t('Current'),
			),
			'#theme' => 'islandora_ontologies_ontology_management_table',
			'rows' => $rows,
		);
		
	$form['submit'] = array(
			'#type' => 'submit',
			'#value' => t('Update Model objects'),
		);
	$form_state['classobject_pid'] =$PID;
	
	return $form;

}

/**
 * Submit handler for model association management form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */

function islandora_ontologies_parsed_class_view_form_submit(array $form, array &$form_state){	
	
// Parse table:
// case 1: selected and status => nothing to do
// case 2: !selected and !status => nothing to do
// case 3: selected and !status = associate model to class
// case 4: !selected and status = remove associate model to class

	$classobject_pid=$form_state['classobject_pid'];

	$models = $form_state['values']['table']['rows'];

	while ($model = current($models)) {
		
		$subject = islandora_object_load($model['modelpid']);
			if (!$subject) {
				drupal_set_message("Fedora object doesn't exist.");
				drupal_not_found();
			exit;
		}
		
		if (($model['selected']) AND !($model['status'])) {
			$subject->relationships->add('info:fedora/fedora-system:def/relations-external#', 'isMemberOf', $classobject_pid);
			drupal_set_message('Relation successfully added');	

		} elseif (!($model['selected']) AND ($model['status'])) {
			
			$resp=$subject->relationships->remove('info:fedora/fedora-system:def/relations-external#', 'isMemberOf', $classobject_pid);
			echo $resp;
			if ($resp) {
				drupal_set_message('Relation successfully removed');
			} else {
				drupal_set_message('Remove relation error');
			}				

		}
		
		next($models);
	}
	
	$form_state['redirect'] = NULL; 
}

/**
* Extract PIDs isMemberOf Class Object
*/
function islandora_ontology_model_object_members() {
	//  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
	$rels_predicate = 'hasModel';
  	
  	$connection = islandora_get_tuque_connection();
  	if ($connection) {
    	$query = <<<EOQ
SELECT ?object
FROM <#ri>
WHERE {
  ?object <fedora-model:$rels_predicate> <info:fedora/fedora-system:ContentModel-3.0>
}
EOQ;
    	$results = $connection->repository->ri->sparqlQuery($query);	
	}
	
	return $results;
}	
	
	