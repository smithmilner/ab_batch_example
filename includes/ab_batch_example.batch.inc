<?php

/*
 *  Load every node and insert the form's text as the new body text.
 */

function ab_batch_example_start($bodytext) {
	// Here we build an array and hand it to the batch api
	$batch = array(
		'operations' => array(),
		'finished' => 'ab_batch_example_finish',
		'title' => t('Rewriting Node Bodies'),
		'init_message' => t('Licking my pen'),
		'progress_message' => t('Processed @current out of @total.'),
		'error_message' => t('AB Example Batch has encountered an error.'),
	);
	// Our functions that batch API will run in order
	$batch['operations'][] = array('ab_batch_example_prepare', array());
	$batch['operations'][] = array('ab_batch_example_rewrite', array($bodytext));

	//tip the first domino!
	batch_set($batch);
}

function ab_batch_example_prepare(&$context) {
	// Initial values that we will use later
	$total = db_query("SELECT count(distinct(nid)) FROM {node}")->fetchField();
	
	//The total number of nodes to rewrite
  $context['results']['total'] = $total;
	//The initial number of nodes we've processed (which is 0)
	$context['results']['processed'] = 0;

	//We're done here so lets tell batch API!
	$context['finished'] = 1;
}

/*
 * rewrite nodes in bunches of 5
 */
function ab_batch_example_rewrite($bodytext, &$context) {
	$limit = 5;
	$offset = $context['results']['processed'];
	
	//A query with a LIMIT statement
	$current_batch = db_query_range("SELECT nid FROM {node}", $offset, $limit);
	foreach($current_batch as $record) {
		_p($record->nid);
		rewrite_this_node($record->nid, $bodytext);
		
		$context['results']['processed']++;	
	}

	//$context['message'] displays on the batch progress page. we can use this to update our progress
  $context['message'] = ab_batch_example_progress($context['results']['processed'], $context['results']['total']);

	//Once the number of nodes we've processed is equal to the total tell batch api we're done!
  $context['finished'] = ($context['results']['processed'] >= $context['results']['total']);
	
}

function ab_batch_example_finish($success, $results, $operations) {
	//You can put finish code here if you like.
	if ($success) {
	    // Here we do something meaningful with the results.
	    $message = $results['processed'].' of '.$results['total'].' nodes rewritten.';
	  }
	  else {
	    // An error occurred.
	    // $operations contains the operations that remained unprocessed.
	    $error_operation = reset($operations);
	    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
	  }
	  drupal_set_message($message);
}

function rewrite_this_node($nid, $bodytext) {
	$node = node_load($nid);

	$node->body[$node->language][0]['value'] = $bodytext;
	$node->body[$node->language][0]['safe_value'] = $bodytext;
	$node->body[$node->language][0]['format'] = 'plain_text';
	$node->revision = 1;
	$node->log = 'This node was programmatically updated at ' . date('c');
	node_save($node);
	
}

function ab_batch_example_progress($current, $total) {
	return number_format(($current * 100) / $total).'% of nodes have been rewritten';
}


?>