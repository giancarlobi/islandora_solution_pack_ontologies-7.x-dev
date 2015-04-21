<?php

function islandora_ontologies_del_relationships_form(array $form, array &$form_state, $islandora_object) {
	module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
	
	$islandora_object_pid = $islandora_object->id;			
	if (!$islandora_object)
	{
		drupal_set_message(t("Islandora Object not found"));
		drupal_not_found();
		exit;	
	}	  
	$currentrelations=islandora_ontologies_get_current_localrelations($islandora_object_pid);
	$currentontologies= islandora_ontologies_get_associated_ontologies($islandora_object);//@see utilities.inc
//--	foreach($currentontologies as $subject=>$ontologies)

	//parse each model assigned to object ($subject) 
	foreach($currentontologies as $subject=>$ontologies){

		// parse each model-class ($ontologies) assigned to object model ($subject)
		foreach($ontologies as $class_PID=>$class_label) {

//--		$posible_relastionships[$subject]=islandora_ontologies_get_possible_localrelations($subject,$ontologies);
		//We are going to use this to also delete the inverse one if they exist, so we don't get orphan relations and a mess.
			$posible_relastionships[$subject][$class_PID]=islandora_ontologies_get_possible_localrelations_bymodelclass($subject,$class_PID,$class_label);
		}
	}
	$rows=array();
	//dpm($currentrelations);
	foreach ($currentrelations as $o){
			
		$target_object_link="";
		$inverse_predicate = "";
		$row=array();
		$row['namespace']['#markup'] = $o['predicate']['namespace']. " (xmlns:".$o['predicate']['alias'].")";
		$row['relationship']['#markup'] = $o['predicate']['value'];
		$predicate=$o['predicate']['namespace'].$o['predicate']['value'];

		if ($o['object']['value']){ //predicate has an object
			try {
				$target_object =islandora_object_load($o['object']['value']);
				$target_object_pid=$o['object']['value'];
				$target_object_link=l(t('@label (@pid)', array('@label' =>$target_object->label, '@pid' =>$target_object_pid)), "islandora/object/{$target_object_pid}/manage/relationships");
				$row['object']['#markup'] = $target_object_link;
				$inverse_ns = NULL;
				$inverse_prop = NULL;
				$row['inverse']['#markup'] = "";
				if ($target_object){
					//Target exists, so lets look for an inverse in the possible array
					//look for an inverse only. 
					foreach ($posible_relastionships as $cmodel => $class_PID){
						foreach ($class_PID as $localrels){
							foreach ($localrels['posiblerels_complete'] as $ontologies=>$rels){
								if (array_key_exists($predicate,$rels)){
									if (isset($rels[$predicate]['inverse'])){
										if (isset($rels[$predicate]['inverse']['ns']) AND isset($rels[$predicate]['inverse']['proplocal'])){
											$inverse_ns = $rels[$predicate]['inverse']['ns'];
											$inverse_prop = $rels[$predicate]['inverse']['proplocal'];
											$row['inverse']['#markup'] = $inverse_ns.$inverse_prop;
										}
									}
								}
							}
						}	
					}
					$row['delete']['#markup'] = l(t('Delete'), islandora_ontologies_get_delete_path($islandora_object_pid, urlencode($o['predicate']['namespace']), $o['predicate']['value'], $o['object']['value'], urlencode($inverse_ns), $inverse_prop));
				} else { //maybe never executed this ELSE
					$row['delete']['#markup'] = "";			
				}
			}
			catch (Exception $e) {
				$target_object_link=t('Object (@pid) does not longer exist or literal',array('@pid'=>$target_object_pid));
				$row['object']['#markup'] = $target_object_link;
				$row['inverse']['#markup'] = "";
				$row['delete']['#markup'] = "";
				//ToDO object no longer exist so we have to remove relationships 
				continue;
			}
			
		} else { //predicate without object
			$row['object']['#markup'] = "";
			$row['inverse']['#markup'] = "";
			$row['delete']['#markup'] = "";
		}
		
	
		
		if ($o['predicate']['namespace'] == 'info:fedora/fedora-system:def/model#') {
			$row['delete']['#markup'] = t('Can\'t delete fedora model.');
		}
		if ($o['predicate']['namespace'] == 'info:fedora/fedora-system:def/relations-external#' and $o['predicate']['value'] == 'isMemberOfCollection') {
			$row['delete']['#markup'] = t('Manage from collection object');
		}
		if ($o['predicate']['namespace'] == 'http://islandora.ca/ontology/relsext#') {
			$row['delete']['#markup'] = t('Manage from object policy');
		}
		
		
		$rows[]=$row;
	};
	
	$form = array(
		'table' => array(
			'#tree' => TRUE,
			'#title' => t('List relationships'),
			'#header' => array(
				t('Namespace'),t('Property'),t('Object'),'Inverse',t('Delete')
			),
			'#theme' => 'islandora_ontologies_ontology_management_table',
			'rows' =>$rows,
		));
		return $form;
	}

	/**
	* @see forked from islandora_relationship_editor 
	*/
function islandora_ontologies_delete_relationships(array $form, array &$form_state, $islandora_object_pid, $namespace, $relationship, $targetPid, $inamespace, $irelationship) {

	try {
		$islandora_object=islandora_object_load($islandora_object_pid);
	}
	catch (Exception $e) {
		return FALSE;
	}
	if (!$islandora_object) {
		drupal_set_message("Fedora object not found.");
		drupal_not_found();
		return;
	}

	if ($islandora_object->relationships->remove(urldecode($namespace), $relationship, $targetPid)) {
		//ok remove direct relationships
		if (!($irelationship == 'NULL') AND !(is_null($irelationship))){
			//inverse remove it			
			try {
				$islandora_object_target=islandora_object_load($targetPid);
			}
			catch (Exception $e) {
				return FALSE;
			}			
			if (!$islandora_object_target) {
				drupal_set_message("Fedora object target not found.");
				drupal_not_found();
				return;
			}			
			
			if ($islandora_object_target->relationships->remove(urldecode($inamespace), $irelationship, $islandora_object_pid)) {
				//ok remove inverse
	  			return array(
				    'object_pid' => array(
				      	'#type' => 'hidden',
				      	'#value' => $islandora_object->id,
				    ),
				    'description' => array(
				      	'#type' => 'markup',
				      	'#prefix' => '<div>',
				      	'#markup' => t('Relationship and inverse deleted'),
				      	'#suffix' => '</div>',
				    ),
				    'back' => array(
				      	'#type' => 'submit',
				      	'#value' => t('Go back'),
				      	'#name' => 'back',
				    ),
				);
			} else {
				//error removing inverse while direct OK
			  	return array(
			    	'object_pid' => array(
			      		'#type' => 'hidden',
			      		'#value' => $islandora_object->id,
			    	),
			    	'description' => array(
			      		'#type' => 'markup',
			      		'#prefix' => '<div>',
			      		'#markup' => t('Relationship deleted. Inverse delete ERROR'),
			      		'#suffix' => '</div>',
			    	),
			    	'back' => array(
			      		'#type' => 'submit',
			      		'#value' => t('Go back'),
			      		'#name' => 'back',
			    	),
			  	);				
			}
		} else {
			//no inverse so OK
  			return array(
			    'object_pid' => array(
			      	'#type' => 'hidden',
			      	'#value' => $islandora_object->id,
			    ),
			    'description' => array(
			      	'#type' => 'markup',
			      	'#prefix' => '<div>',
			      	'#markup' => t('Relationship deleted'),
			      	'#suffix' => '</div>',
			    ),
			    'back' => array(
			      	'#type' => 'submit',
			      	'#value' => t('Go back'),
			      	'#name' => 'back',
			    ),
			);			
		}
	} else {
	  	//error removing direct relationship so exit without try inverse
	  	return array(
	    	'object_pid' => array(
	      		'#type' => 'hidden',
	      		'#value' => $islandora_object->id,
	    	),
	    	'description' => array(
	      		'#type' => 'markup',
	      		'#prefix' => '<div>',
	      		'#markup' => t('Relationship delete ERROR'),
	      		'#suffix' => '</div>',
	    	),
	    	'back' => array(
	      		'#type' => 'submit',
	      		'#value' => t('Go back'),
	      		'#name' => 'back',
	    	),
	  	);
	}

}

function islandora_ontologies_delete_relationships_submit(array $form, array &$form_state) {

	$form_state['redirect'] = 'islandora/object/' . $form_state['values']['object_pid'] . '/manage/relationships';
}
