<?php
/*
* @file
* Holds  functions to manage ontologies and class objects for islandora_ontologies.
*/
$module_path = drupal_get_path('module', 'islandora_ontologies');
include_once($module_path.'/arc2/ARC2.php');
include_once($module_path.'/Graphite/Graphite.php');

/**
* This function will read the ONTOLOGY Stream from an object and display a table to add class objects to matching owl:classes
*
*/

function islandora_ontologies_parsed_vocabulary_view_form(array $form, array &$form_state, $islandora_object_pid){
	module_load_include('inc', 'islandora', 'includes/utilities');

	$islandora_object = islandora_object_load($islandora_object_pid);
	$datastream = $islandora_object['ONTOLOGY'];
	if (!$datastream)
	{
		drupal_set_message(t("Fedora Ontoloy object doesn't have an ONTOLOGY stream."));
		drupal_not_found();
		exit;
	}

	$PID=$islandora_object->id;
	$fedora_url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
	$xmlfilelink = $fedora_url."/objects/".$PID."/datastreams/ONTOLOGY/content";
	$graph = new Graphite();
	$result_load = $graph->load($xmlfilelink);
	if (!$result_load)
	{
		drupal_set_message(t("ONTOLOGY empty."));
		drupal_not_found();
		exit;
	}
	$defined_classes=array();
	foreach($graph->allOfType('owl:Class') as $resource_list)
	{
		$defined_classes[$resource_list->uri]=$resource_list->uri;
	}
	$rows = array();
	
	$classObjects = islandora_ontology_object_get_classes($PID);
	
	foreach ($defined_classes as $defined_class) {

		if (array_key_exists($defined_class,$classObjects)) {

			$class_object_pid=$classObjects[$defined_class];
			$existingassociation=1;
			$label = $defined_class;
		} else {
			$class_object_pid="-";
			$existingassociation=0;
			$label = $defined_class;	
		}			
		
		$rows[$defined_class] = array(
			'selected' => array(
				'#type' => 'checkbox',
				'#default_value' => $existingassociation,
			),
			'class_label' => array(
				'#markup' => l(t('@label', array('@label' => $label)), "islandora/object/{$class_object_pid}"),
			),
			'classpid' => array(
				'#type' => 'textfield',
				'#default_value' => $class_object_pid,
				'#disabled' => TRUE,
				'#size' => 20,
			),			
		);
	}
	$form['help'] = array(
			'#type' => 'item',
			'#markup' => l(t('About Ontology'), '', array()),
		);
	$form['table'] = array(
			'#tree' => TRUE,
			'#header' => array(
				array('class' => array('select-all')), t('Class'), 'PID',
			),
			'#theme' => 'islandora_ontologies_ontology_management_table',
			'rows' => $rows,
		);
		
	$form['submit'] = array(
			'#type' => 'submit',
			'#value' => t('Update Class objects'),
		);
	$form_state['ontologyobject_pid'] =$PID;
	return $form;
}

/**
 * Submit handler for ontology class association management form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */

function islandora_ontologies_parsed_vocabulary_view_form_submit(array $form, array &$form_state){	
	
// Parse table:
// case 1: flag and PID = class object exists, nothing to do
// case 2: !flag and !PID = nothing to do
// case 3: flag and !PID = class object doesn't exist, class object to ingest
// case 4: !flag and PID = class object exists, class object to remove if no relationships

	$ontologyobject_pid=$form_state['ontologyobject_pid'];

	$classes = $form_state['values']['table']['rows'];
	while ($defclass = current($classes)) {
		
		if (($defclass['selected']) AND ($defclass['classpid'] == '-')) {

			echo key($classes).' : '.$defclass['selected'].' - '.$defclass['classpid'].' ingest new object <br />';
			
			$connection = islandora_get_tuque_connection();
			if ($connection) {

  				$class_object = $connection->repository->constructObject('ontologies');
  				$class_object->owner = 'fedoraAdmin';
  				$class_object->label = key($classes);
  				$class_object->models = 'ontologies:classCModel';
  				$class_object->relationships->add(FEDORA_RELS_EXT_URI, 'isConstituentOf', $ontologyobject_pid);
				if ($connection->repository->ingestObject($class_object)) {
					drupal_set_message(t("Class object ".key($classes)." ingested."));
				} else {
					drupal_set_message(t("Class object ".key($classes)." ingesting error."),'error');
				}
				
			} else {
				drupal_set_message(t("Class object ".key($classes)." ingesting error."),'error');
			}

		} elseif ((!$defclass['selected']) AND ($defclass['classpid'] != '-')) {

			echo key($classes).' : '.$defclass['selected'].' - '.$defclass['classpid'].' remove object <br />';
			$members = islandora_ontology_class_object_members($defclass['classpid']);
			if (count($members)){
				drupal_set_message(t("Class object ".key($classes)." still has ".count($members)." relationships."),'error');
			} else {
				$connection = islandora_get_tuque_connection();
				if ($connection) {
					if ($connection->repository->purgeObject($defclass['classpid'])) {
						drupal_set_message(t("Class object ".key($classes)." purged."));
					} else {
						drupal_set_message(t("Class object ".key($classes)." purging error."),'error');
					}
				} else {
					drupal_set_message(t("Class object ".key($classes)." purging error."),'error');
				}
			}
		}
		next($classes);
	}
	$form_state['redirect'] = NULL; 
}
	/**
	 * Manage Object Relations local task.
	 *
	 * Defines the actions to appear in the Relations section of the Manage tab.
	 *
	 * @param AbstractObject $object
	 *   The collection object.
	 *
	 * @return array
	 *   A renderable array.
	 */
function islandora_ontologies_manage_relations(AbstractObject $object) {
  	module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
	module_load_include('inc', 'islandora_ontologies', 'includes/add_relation');
	module_load_include('inc', 'islandora_ontologies', 'includes/del_relation');
	
	  $return_form = array(
	    'manage_relations' => array(),
	  );
	  $data = islandora_ontologies_manage_relations_form($return_form, $object);
	  
	  $data['manage_relations']['#type'] = 'vertical_tabs';
	  return $data;
	}
	/**
	 * Implements islandora_basic_collection_build_manage_object_hook().
	 *
	 * @param array $form_state
	 *   The Form state of the form.
	 * @param AbstractObject $object
	 *   The Islandora object being processed.
	 *
	 * @return array
	 *   Returning the built form.
	 */
function islandora_ontologies_manage_relations_form(array $form_state, AbstractObject $object) {
	  // Defines accessible elements.
  	
	  $access = array(
	    'add_relationships' => islandora_object_access_callback(ISLANDORA_MANAGE_PROPERTIES, $object),
	    'del_relationships' => islandora_object_access_callback(ISLANDORA_MANAGE_PROPERTIES, $object),
	  );
	 
	  // Build forms for only the accessible elements.
	  foreach ($access as $element => $accessible) {
	    $form = "islandora_ontologies_{$element}_form";
		
	    $forms[$element] = $accessible ? drupal_get_form($form, $object) : NULL;
	  }
	  
	  $form_state['manage_relations']['add_relationships'] = array(
	    '#id' => 'add-relations',
	    '#group' => 'manage_relations',
	    '#access' => $access['add_relationships'],
	    '#type' => 'fieldset',
	    '#title' => t('Add Relations'),
	    'form' => $forms['add_relationships'],
	  );
	  $form_state['manage_relations']['del_relationships'] = array(
	    '#id' => 'delete-relations',
	    '#group' => 'manage_relations',
	    '#access' => $access['del_relationships'],
	    '#type' => 'fieldset',
	    '#title' => t('Manage existing relationships'),
	    'form' => $forms['del_relationships'],
	  );
	  return $form_state;
	}	

	
/**
* Return an array of pids that are classes of a ontology object.
*/
function islandora_ontology_object_get_classes($pid) {
//  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
	$rels_predicate = 'isConstituentOf';

  $objects = array();

  $connection = islandora_get_tuque_connection();
  $escaped_pid = str_replace(':', '_', $pid);
  if ($connection) {
    $query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
SELECT ?object ?title
FROM <#ri>
WHERE {
  ?object <fedora-model:label> ?title ;
          <fedora-rels-ext:$rels_predicate> <info:fedora/$pid> .

}
EOQ;
    $results = $connection->repository->ri->sparqlQuery($query);

    foreach ($results as $result) {
      $object = islandora_object_load($result['object']['value']);
      if (islandora_object_access(ISLANDORA_VIEW_OBJECTS, $object)) {
        $objects[$result['title']['value']] = $result['object']['value'];
      }
    }
  }

  return $objects;
}

/**
* Extract PIDs isMemberOf Class Object
*/
function islandora_ontology_class_object_members($pid) {
	//  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
	$rels_predicate = 'isMemberOf';
  	
  	$connection = islandora_get_tuque_connection();
  	$escaped_pid = str_replace(':', '_', $pid);
  	if ($connection) {
    	$query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
SELECT ?object
FROM <#ri>
WHERE {
  ?object <fedora-rels-ext:$rels_predicate> <info:fedora/$pid>
}
EOQ;
    	$results = $connection->repository->ri->sparqlQuery($query);	
	}
	
	return $results;
}	
	
	
	
	