<?php

function islandora_ontologies_add_relationships_form(array $form, array &$form_state, $islandora_object) {
	module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
	form_load_include($form_state, 'inc', 'islandora_ontologies', 'includes/add_relation');
	//Needed because we have to ways of calling this form and inside the wrapper function 
	//islandora_ontologies_manage_relations the link to the embeded and needed functions in this file get`s lost.
	
	// why ?
	//islandora_ontologies_get_all_for_cmodel('biodiversity:OccurrenceCModel');
	//***************************
	
	
	if (!$islandora_object)
	{
		drupal_set_message(t("Islandora Object not found"));
		drupal_not_found();
		exit;	
	}	  
	$islandora_object_pid = $islandora_object->id;		
	//We don´t want to reprocess every bit every time we re-render or sumbit the form. 
	//So store the processed ontology values on form_state		
	$select_field="";

	if (!isset($form_state['islandora_ontologies']))
	{
		
		$restrictionsOnOntologies=array();
		$posible_relastionships=array();
		$currentrelationsfiltered=array();
		$options_subjectCmodel=array();
		$currentontologies=array();

		$currentontologies= islandora_ontologies_get_associated_ontologies($islandora_object);//@see utilities.inc
		//$currentontologies = for each model assigned to object list all model-class associations (class object PID + label)
		//Array ( [islandora:sp_large_image_cmodel] => 
		//	Array ( 	[ontologies:23] => http://aims.fao.org/aos/v1.0/c_image 
		//              [ontologies:27] => info:fedora/biodiversity:OccurrenceCModel 
		//				[ontologies:22] => http://aims.fao.org/aos/v1.0/c_noun ) )
				
		
		$currentrelations=islandora_ontologies_get_current_localrelations($islandora_object_pid);//@see utilities.inc
		//$currentrelations = all existing relationships with the object as subject
		//Array ( 	[2] => 
		//				Array ( [predicate] => 
		//							Array ( [value] => hasPathogen [alias] => [namespace] => http://aims.fao.org/aos/v1.0/ ) 
		//						[object] => 
		//							Array ( [literal] => [value] => I-TO0762:9 ) ) 
		//			[3] => 
		//				Array ( [predicate] => 
		//							Array ( [value] => correspondsTo [alias] => [namespace] => http://aims.fao.org/aos/v1.0/ ) 
		//						[object] => 
		//							Array ( [literal] => [value] => A-TO0762:2 ) ) 
		//			[4] => 
		//				Array ( [predicate] => 
		//							Array ( [value] => hasPathogen [alias] => [namespace] => http://aims.fao.org/aos/v1.0/ ) 
		//						[object] => 
		//							Array ( [literal] => [value] => I-TO0762:22 ) ) 
		//			[5] => 
		//				Array ( [predicate] => 
		//							Array ( [value] => hasPathogen [alias] => [namespace] => http://aims.fao.org/aos/v1.0/ ) 
		//						[object] => 
		//							Array ( [literal] => [value] => I-TO0762:8 ) ) ) 
				
/* TODO: if NULL $currentontologies => warning without model-Class association rels are not permited */


		//parse each model assigned to object ($subject) 
		foreach($currentontologies as $subject=>$ontologies)
		{
		
			// $subject: islandora:sp_large_image_cmodel 
			// $ontologies:
			//	Array ( 	[ontologies:23] => http://aims.fao.org/aos/v1.0/c_image 
			//              [ontologies:27] => info:fedora/biodiversity:OccurrenceCModel 
			//				[ontologies:22] => http://aims.fao.org/aos/v1.0/c_noun ) )			

//			$posible_relastionships[$subject]=islandora_ontologies_get_possible_localrelations($subject,$ontologies);
			// all relationships without restriction
			
			
			$currentrelationsfiltered[$subject]=islandora_ontologies_filter_existing_relations($subject,$ontologies,$currentrelations);	
			// only current relations included in object ontologies


			// parse each model-class ($ontologies) assigned to object model ($subject)
			foreach($ontologies as $class_PID=>$class_label) {
				
				$ontology_objectpid=islandora_ontologies_get_ontology_for_class($class_PID);

				$posible_relastionships[$subject][$class_PID]=islandora_ontologies_get_possible_localrelations_bymodelclass($subject,$class_PID,$class_label);

				$restrictionsOnOntologies[$subject][$ontology_objectpid][$class_PID]=islandora_ontologies_get_restricted_properties($ontology_objectpid,$class_label,$currentrelationsfiltered[$subject][$ontology_objectpid]);

			}

//print_r($posible_relastionships); exit;

/*
 * $posible_relastionships=
Array ( [islandora:sp_large_image_cmodel] => Array ( 
     		[ontologies:23] => Array (
          		[posiblerels_complete] => Array ( 
					[ontologies:1] => Array ( 
			  			[http://aims.fao.org/aos/v1.0/actsUpon] => Array ( 
							[label] => Acts upon 
							[inverse] => Array ( 
								[ns] => http://aims.fao.org/aos/v1.0/ 
								[proplocal] => isActedUponBy ) 
							[inverseof] => Array ( 
								[ns] => http://aims.fao.org/aos/v1.0/ 
								[proplocal] => isActedUponBy ) 
							[ns] => http://aims.fao.org/aos/v1.0/ 
							[comment] => 
							[proplocal] => actsUpon 
							[alias] => ) 
		
			  			[http://aims.fao.org/aos/v1.0/affects] => Array ( 
...
           		[posiblerels_asformoptions] => Array ( 
             		[AOS] => Array ( 
                		[ontologies:1 http://aims.fao.org/aos/v1.0/actsUpon] => Acts upon 
               			[ontologies:1 http://aims.fao.org/aos/v1.0/affects] => Affects 

*/

/*
 * $restrictionsOnOntologies=

Array ( [islandora:sp_large_image_cmodel] => Array ( 

	[ontologies:1] => Array ( 

		[http://aims.fao.org/aos/v1.0/c_image] => Array ( 
		
		) 

		[http://aims.fao.org/aos/v1.0/c_noun] => Array ( 
			[http://aims.fao.org/aos/v1.0/hasImage] => Array ( 
				[msg] => Array ( 
					[0] => Cardinality on http://aims.fao.org/aos/v1.0/hasImage complaint with ontology ) 
				[onClass] => http://aims.fao.org/aos/v1.0/c_image 
				[inverse] => Array ( 
					[ns] => http://aims.fao.org/aos/v1.0/ 
					[localname] => isImageOf ) 
				[inversof] => http://aims.fao.org/aos/v1.0/isImageOf 
				[allowsmore] => 1 
				[ns] => http://aims.fao.org/aos/v1.0/ 
				[localname] => hasImage 
			) 
		) 
	) 

*/
		
			$value_subjectCmodel=$subject;//Lets set any as default
		}
		
		$options_subjectCmodel_tmp=array_keys($currentontologies);
		if (sizeof($options_subjectCmodel_tmp)>0)
		{
			$options_subjectCmodel=array_combine($options_subjectCmodel_tmp,$options_subjectCmodel_tmp);//keys and values the same
		}
		$form_state['islandora_ontologies']['current']=$currentrelations;
		$form_state['islandora_ontologies']['restrictions']=$restrictionsOnOntologies;
		$form_state['islandora_ontologies']['posible_relations']=$posible_relastionships;
		$form_state['islandora_ontologies']['options_subjectcmodel']=$options_subjectCmodel;
		$form_state['islandora_ontologies']['current_ontologies']=$currentontologies;
	}

	$value_relationship=!empty($form_state['values']['relationship']) ? $form_state['values']['relationship'] : "";	
	$value_subjectCmodel=!empty($form_state['values']['subjectCmodel_select']) ? $form_state['values']['subjectCmodel_select'] : "";
	$value_subjectClass=!empty($form_state['values']['subjectClass']) ? $form_state['values']['subjectClass'] : "";
	
	$form = array();
	$form['this'] = array(
		'#title' => t('This Object'),
		'#type' => 'textfield',
		'#value' => $islandora_object_pid,
		'#disabled' => TRUE,
	);



	$form['subject'] = array(
		'#type' => 'fieldset',
		'#title' => 'Subject Class(CMODEL)',
	);
	$form['subject']['subjectCmodel'] =array(
		'#type' => 'hidden',
		'#value'=> $value_subjectCmodel,
		'#default_value'=> $value_subjectCmodel,
	);

	if (sizeof($form_state['islandora_ontologies']['posible_relations'])>0)  
	//means we have more than one CMODEL as subject, so me must deal with restrictions based on that...
	{
		$form['subject']['subjectCmodel_select'] = array(
			'#type' => 'select',
			'#title' => t('Content Models for this Object'),
			'#options' => $form_state['islandora_ontologies']['options_subjectcmodel'],
			'#required' => TRUE,
			'#ajax' => array(
				'method' => 'replace',
				'effect' => 'fade',
				'event' => 'change',
				'callback' => 'relationship_subject_ajax_callback',//I woud really love ajax callbacks could use parameters... 
			),
		);	
		
		$form['subject']['subjectClass'] = array(
			'#type' => 'select',
			'#title' => t('Class for this Content Models'),
			'#options' => select_class_values($value_subjectCmodel,$form_state['islandora_ontologies']['current_ontologies']),
			'#required' => TRUE,
			'#ajax' => array(
				'method' => 'replace',
				'effect' => 'fade',
				'event' => 'change',
				'callback' => 'class_subject_ajax_callback',//I woud really love ajax callbacks could use parameters... 
			), 
			'#prefix' => '<div id="class_select_replace">',
			'#suffix' => '</div>',
		);
	
		$form['rels'] = array(
			'#type' => 'fieldset',
			'#title' => 'Relationship',
			'#prefix' => '<div id="relationship_replace">',
			'#suffix' => '</div>',
		);
   	   
		$form['rels']['relationship'] = array(
			'#type' => 'select',
			'#title' => t('Relationship'),
			'#options' => select_relationsship_values($value_subjectCmodel,$value_subjectClass,$form_state['islandora_ontologies']['posible_relations']),
			'#required' => TRUE,
			'#ajax' => array(
				'method' => 'replace',
				'effect' => 'fade',
				'event' => 'change',
				'callback' => 'relationship_ajax_callback',//I woud really love ajax callbacks could use parameters... 
			),
			'#prefix' => '<div id="relationship_select_replace">',
			'#suffix' => '</div>',
		);
	

		$form['rels']['description'] = array(
			'#markup' => textfield_description_value($value_relationship,$value_subjectCmodel,$value_subjectClass,$form_state['islandora_ontologies']['posible_relations']),
			'#prefix' => '<div id="textfield_description_replace">',
			'#suffix' => '</div>',
		);

		$form['ns'] = array(
			'#type' => 'fieldset',
			'#title' => 'Namespace',
			'#prefix' => '<div id="fieldset_ns_replace">',
			'#suffix' => '</div>',
		);

		$form['ns']['namespace'] = array(
			'#type' => 'textfield',
			'#title' => t('Namespace'),
			'#disabled' => TRUE,
			'#value' => textfield_namespace_value($value_relationship,$value_subjectCmodel,$value_subjectClass,$form_state['islandora_ontologies']['posible_relations']),
		);

	/*$form['ns']['curr_alias'] = array(
		'#type' => 'textfield',
		'#title' => t('Namespace alias'),
		'#disabled' => TRUE,
		'#value' => "",//isset($form_state['values']['relationship']) ? ns_has_alias($value_relationship, $object_pid) : "",
		);*/

		/*$form['ns']['add_new_alias'] = array(
			'#type' => 'checkbox',
			'#disabled' => TRUE,
			'#title' => t('Add NEW alias'),
			'#default_value' => FALSE,
			'#value' => "",//(isset($form_state['values']['relationship']) && !(ns_has_alias($value_relationship, $object_pid))) ? TRUE : FALSE,
			);*/

			/*$form['ns']['alias'] = array(
				'#type' => 'textfield',
				'#title' => t('NEW Namespace alias (blank for inline namespace)'),
				'#disabled' => "",//(isset($form_state['values']['relationship']) && !(ns_has_alias($value_relationship, $object_pid))) ? FALSE : TRUE,
				);*/

		$form['obj'] = array(
			'#type' => 'fieldset',
			'#title' => 'Object',
			'#prefix' => '<div id="fieldset_objtarget_replace">',
			'#suffix' => '</div>',
  
		);
		
		$form['obj']['msg'] = array(
			'#markup'=>t('<b>Select a relationship first</b>'),
			'#access'=>empty($value_relationship),
		);
		
		$form['obj']['target'] = array(
			'#type' => 'textfield',
			'#title' => t('Target object'),
			'#required' => TRUE,
			'#autocomplete_path' => textfield_autocomplete_value($value_relationship,$form_state['islandora_ontologies']['restrictions'],$value_subjectCmodel,$value_subjectClass),
			'#access'=>!empty($value_relationship),
			//Autocomplete_path, must be dinamic added to allow only objects of restricted class for this property 
		);
		
		$form['obj']['restriction']=array(
			'#items' => items_restrictions_value($value_relationship,$form_state['islandora_ontologies']['restrictions'],$value_subjectCmodel,$value_subjectClass,$form_state['islandora_ontologies']['posible_relations']),
			'#theme' => 'item_list',
		);

		$form['submit'] = array(
			'#type' => 'submit',
			'#value' => t('submit'),
		);
	}
	return $form;
}


function relationship_subject_ajax_callback(&$form, $form_state) {
	$ajax = array( '#type' => 'ajax', '#commands' =>'');
	if (!empty($form_state['values']['subjectCmodel_select'])) 
	{
 
		$ajax['#commands']=array(
			ajax_command_replace("#class_select_replace", render($form['subject']['subjectClass'])),
			ajax_command_replace("#relationship_replace", render($form['rels'])),
			ajax_command_replace("#fieldset_ns_replace", render($form['ns'])),
			ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
			//ajax_command_invoke("#edit-obj",'show'),
		);
	}
	else
	{
		$ajax['#commands']=array(
			ajax_command_replace("#class_select_replace", render($form['subject']['subjectClass'])),
			ajax_command_replace("#relationship_replace", render($form['rels'])),
			ajax_command_replace("#fieldset_ns_replace", render($form['ns'])),
			ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
		);
	}
	return $ajax;
}

function class_subject_ajax_callback(&$form, $form_state) {
	$ajax = array( '#type' => 'ajax', '#commands' =>'');
	if (!empty($form_state['values']['subjectClass'])) 
	{
		$ajax['#commands']=array(
			ajax_command_replace("#relationship_replace", render($form['rels'])),
			ajax_command_replace("#fieldset_ns_replace", render($form['ns'])),
			ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
		);
	}
	else
	{
		$ajax['#commands']=array(
			ajax_command_replace("#relationship_replace", render($form['rels'])),
			ajax_command_replace("#fieldset_ns_replace", render($form['ns'])),
			ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
		);
	}
	return $ajax;
}
						
			function select_class_values($value_subjectCmodel,$ontologies) {
	
				if (!empty($value_subjectCmodel)) {
					$options=$ontologies[$value_subjectCmodel];
  
				}
				return empty($options) ? array() : $options;
			};
			
			
			function relationship_ajax_callback(&$form, $form_state) {
				$ajax = array( '#type' => 'ajax', '#commands' =>'');
  
				if (!empty($form_state['values']['relationship'])) 
				{
					$form['obj']['target']['#value']="";//lets clear this;	 
					$ajax['#commands']=array(
						ajax_command_replace("#textfield_description_replace", render($form['rels']['description'])),
						//ajax_command_replace("#fieldset_symm_replace", render($form['symmetric'])),
						ajax_command_replace("#fieldset_ns_replace", render($form['ns'])),
						ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
						//ajax_command_invoke("#edit-obj",'show'),
	
					);
				}
				else
				{
					$form['obj']['target']['#value']="";	
					$ajax['#commands']=array(
						ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
					);
				}
				return $ajax;
			}
			
			function select_relationsship_values($value_subjectCmodel,$value_subjectClass,$possible_relations) {
	
				if (!empty($value_subjectClass)) {
					$options=$possible_relations[$value_subjectCmodel][$value_subjectClass]['posiblerels_asformoptions'];

				}
				return empty($options) ? array() : $options;
			};

			function textfield_namespace_value($value_relationship,$value_subjectCmodel,$value_subjectClass,$possible_relations) {
				if (!empty($value_relationship)) {
					list($ontology,$property)=explode(" ",$value_relationship);
					//dpm($possible_relations[$subject]['posiblerels_complete']);
					$value_ns = $possible_relations[$value_subjectCmodel][$value_subjectClass]['posiblerels_complete'][$ontology][$property]['ns'];
				}
				return empty($value_ns) ? "No namespace" : $value_ns;
			};

			function textfield_description_value($value_relationship,$value_subjectCmodel,$value_subjectClass,$possible_relations) {
  
				if (!empty($value_relationship)) {
					list($ontology,$property)=explode(" ",$value_relationship);
					$value_description=$possible_relations[$value_subjectCmodel][$value_subjectClass]['posiblerels_complete'][$ontology][$property]['comment'];
				}
				return empty($value_description) ? "No description" : $value_description;
			}
			function textfield_autocomplete_value($value_relationship,$restrictionson_properties,$subject,$value_subjectClass) {
				$autocomplete_target='islandora/relationships/autocomplete/target/';
				if (!empty($value_relationship)) {
					list($ontology,$property)=explode(" ",$value_relationship);
					if ($restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['onClass']) //= class_label !!!!
					{
						$autocomplete_target=$autocomplete_target.urlencode($restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['onClass'])."/".$ontology."/";
					}
					else
					{
						$autocomplete_target=$autocomplete_target."/".$ontology."/";	
					}  	
				}
				return $autocomplete_target;
			}

			function items_restrictions_value($value_relationship,$restrictionson_properties,$subject,$value_subjectClass,$possible_relations) {
				$value_restrictions=array();
				if (!empty($value_relationship)) {
					list($ontology,$property)=explode(" ",$value_relationship);


					if ($restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['onClass']) //= class_label !!!!
					{
						$value_restrictions[]=t('<b>Target class constraint: '.$restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['onClass'].'</b>');
					}
					else
					{
						$value_restrictions[]=t('<b>Target without constrains</b>');	
					} 

					if ($possible_relations[$subject][$value_subjectClass]['posiblerels_complete'][$ontology][$property]['inverse'])
//					if ($restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['inverse'])
					{	
//						$value_restrictions[]=t('<b>This relation implements an inverse one: </b>').$restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['inverse']['localname'];	
						$value_restrictions[]=t('<b>This relation implements an inverse one: </b>').$possible_relations[$subject][$value_subjectClass]['posiblerels_complete'][$ontology][$property]['inverse']['proplocal'];	
					}	elseif ($possible_relations[$subject][$value_subjectClass]['posiblerels_complete'][$ontology][$property]['inverseof'])
					{
//						$value_restrictions[]=t('<b>This relation is the inverse of: </b>').$restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['inversof'].t('<p>If you wan´t both relations to be added, then you must do this from the target Object</p>');	
						$value_restrictions[]=t('<b>This relation is the inverse of: </b>').$possible_relations[$subject][$value_subjectClass]['posiblerels_complete'][$ontology][$property]['inverseof']['proplocal'].t('<p>If you wan´t both relations to be added, then you must do this from the target Object</p>');	
					}

/*
					if ($restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['allowsmore'])
						{
					$value_restrictions[]=t('<b>You can add more of this relations</b>');
					$value_restrictions[]=t('<b>To Objects of type: </b>').$restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['onClass'];
						}
					else
						{
						$value_restrictions[]=t('<b>You have reached the limit for this relation!</b>');	
						}
					
					if ($restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['inverse'])
						{
							
						$value_restrictions[]=t('<b>This relation implements an inverse one: </b>').$restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['inverse']['localname'];	
						}
					if ($restrictionson_properties[$subject][$ontology][$property]['inversof'])
						{
						$value_restrictions[]=t('<b>This relation is the inverse of: </b>').$restrictionson_properties[$subject][$ontology][$value_subjectClass][$property]['inversof'].t('<p>If you wan´t both relations to be added, then you must do this from the target Object</p>');	
						}	
*/

				}


				return $value_restrictions;
			}

			

function islandora_ontologies_add_relationships_form_submit($form, &$form_state) {

	$subject = islandora_object_load($form_state['values']['this']);
	//Subject in the sense of the actual relation. We used subject before what in the meaning of subject(class) in the ontologi
	$possible_relations=$form_state['islandora_ontologies']['posible_relations'];
	$current_relations=$form_state['islandora_ontologies']['current'];
	$restrictionson_properties=$form_state['islandora_ontologies']['restrictions'];

	if (!$subject) {
		form_set_error('target', "Subject error: Fedora object doesn't exist.");
		$form_state['redirect'] = FALSE;
		return ;
	}


	$object = islandora_object_load($form_state['values']['target']);
	if (!$object) {
		form_set_error('target', "Object error: Fedora object doesn't exist.");
		$form_state['redirect'] = FALSE;
		return ;
	}
  
	$subjectCMODEL=$form_state['values']['subjectCmodel_select'];
	$subjectCLASS=$form_state['values']['subjectClass'];
	list($ontology,$property)=explode(" ",$form_state['values']['relationship']);
	$rel_ns = $possible_relations[$subjectCMODEL][$subjectCLASS]['posiblerels_complete'][$ontology][$property]['ns'];
	$rel_name =  $possible_relations[$subjectCMODEL][$subjectCLASS]['posiblerels_complete'][$ontology][$property]['proplocal'];

	if (empty($rel_ns) || empty($rel_name))
	{
		form_set_error('relationship', "Relationship error: No property or namespace absent");
		$form_state['redirect'] = FALSE;
		return ;	
	}  

	$objectontologies=array();
	$objectontologies= islandora_ontologies_get_associated_ontologies($object);//@see utilities.inc
	//$currentontologies = for each model assigned to object list all model-class associations (class object PID + label)
	//Array ( [islandora:sp_large_image_cmodel] => 
	//	Array ( 	[ontologies:23] => http://aims.fao.org/aos/v1.0/c_image 
	//              [ontologies:27] => info:fedora/biodiversity:OccurrenceCModel 
	//				[ontologies:22] => http://aims.fao.org/aos/v1.0/c_noun ) )
	
	if (isset($restrictionson_properties[$subjectCMODEL][$ontology][$subjectCLASS][$property]['onClass'])){	
		$target_class= FALSE;
		foreach($objectontologies as $object_model=>$object_classes){
		
			// $object_model: islandora:sp_large_image_cmodel 
			// $object_classes:
			//	Array ( 	[ontologies:23] => http://aims.fao.org/aos/v1.0/c_image 
			//              [ontologies:27] => info:fedora/biodiversity:OccurrenceCModel 
			//				[ontologies:22] => http://aims.fao.org/aos/v1.0/c_noun ) )			

			// parse each model-class ($object_classes) assigned to object model ($object_model)
			foreach($object_classes as $class_PID=>$class_label){
				if ($class_label == $restrictionson_properties[$subjectCMODEL][$ontology][$subjectCLASS][$property]['onClass']){
					$target_class= TRUE;
				}
			}
		}
	}else{
		$target_class= TRUE;
	}

	if (!$target_class)
	{
		form_set_error('target', "Target model error: class constraint not satisfied");
		$form_state['redirect'] = FALSE;
		return ;	
	} 


    if (isset($possible_relations[$subjectCMODEL][$subjectCLASS]['posiblerels_complete'][$ontology][$property]['inverse']))
    //we also have an inverse. Just add, that´s the reason they are defined!
		{
		if (is_array($possible_relations[$subjectCMODEL][$subjectCLASS]['posiblerels_complete'][$ontology][$property]['inverse'])>0)	
			{
			$relinv_ns = isset($possible_relations[$subjectCMODEL][$subjectCLASS]['posiblerels_complete'][$ontology][$property]['inverse']['ns'])?$possible_relations[$subjectCMODEL][$subjectCLASS]['posiblerels_complete'][$ontology][$property]['inverse']['ns']:NULL;	
			$relinv_name= isset($possible_relations[$subjectCMODEL][$subjectCLASS]['posiblerels_complete'][$ontology][$property]['inverse']['proplocal'])?$possible_relations[$subjectCMODEL][$subjectCLASS]['posiblerels_complete'][$ontology][$property]['inverse']['proplocal']:NULL;	
			}	
		}


	//Don´t allow duplicated relations please!	
	foreach ($current_relations as $key=>$relation)
	{
		if (($relation['object']['value']==$form_state['values']['target'])&&($relation['predicate']['value']==$rel_name) && ($relation['predicate']['namespace']==$rel_ns))
		{
			form_set_error('relationship', t("Relationship error: this relation is already present"));
			$form_state['redirect'] = FALSE;
			return ;
		}

			
	}	






/**** only for TEST

	form_set_error('target', "Subject error: TEST no submit - ".$relinv_ns.$relinv_name);
	$form_state['redirect'] = FALSE;
	return ;


****/

  







	$subject->relationships->add($rel_ns, $rel_name, $form_state['values']['target']);
	drupal_set_message('Relation successfully added');
	
	//Let`s try our inverse
	if (!empty($relinv_ns) && !empty($relinv_name))
	{
	$invrels=$object->relationships->get($relinv_ns,$relinv_name,$subject->id);
	if (count($invrels)==0)//means we don´t have already this relation in place...we can proceed
		{
			$object->relationships->add($relinv_ns,$relinv_name,$subject->id);
			drupal_set_message('Inverse relation successfully added');
		}
	
	
	}
$form_state['redirect'] = "islandora/object/{$subject->id}/manage/relationships";
}




