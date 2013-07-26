<?php 
class Config {

	function __construct () {
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.config.js');
	}

	private function getIntroduction () {
		return t('Here you will be able to choose which fields you want to show when the user opens the VisualScience module. Note that every field that is in the minimized table will also be in the full one. And the last name and full name are required in the mini table.<br /> <b>At the moment, the full table option is not used, but we are working hard on it.<b>');

	}

	private function createRows ($list, $oldList) {
		$rows = array();
		foreach ($list as $l) {
			$mini = '';
			$full = ''; 
			$first = '';
			$last = '';
			if (isset($oldList[$l])) {
				$actual = $oldList[$l];
				$mini = $actual['mini'] == 1 ? 'checked': '';
				$full = $actual['full'] == 1 ? 'checked': '';
				$first = $actual['first'] == 1 ? 'checked': '';
				$last = $actual['last'] == 1 ? 'checked': '';
			}
			$row = array(
				$l,
				'<input class="mini" type="checkbox" name="'.$l.'-mini" value="1" '.$mini.' />',  
				'<input class="full" type="checkbox" name="'.$l.'-full" value="1" '.$full.' />',  
				'<input class="first" type="radio" name="first" value="'.$l.'" '.$first.' />',  
				'<input class="last" type="radio" name="last" value="'.$l.'" '.$last.' />',
				);
			array_push($rows, $row);
		}
		return $rows;
	}

	private function createFieldsTable ($fields, $oldFields) {
		$header = array(t('Field Name'), 
			t('Show in minimized table ?'), 
			t('Show in full table ?'), 
			t('Which one is the First Name field ?'), 
			t('Which one is the Last Name field ?'),
			);
		$rows = $this->createRows($fields, $oldFields);
		return theme('table', array('header' => $header, 'rows' => $rows));
	}

	private function createSaveButton () {
		$button = '<input type="submit" value="'.t('Save').'" class="form-submit" />';
		return $button;
	}

	private function emptyOldValues () {
		$query = db_delete('visualscience_search_config');
		$query->execute();
	}

	private function insertIntoSearchConfig ($name, $mini, $full, $first, $last, $field = 0) {
		$table = 'visualscience_search_config';
		$query = db_insert($table)->fields(array(
			'name' => $name,
			'mini' => $mini,
			'full' => $full,
			'first' => $first,
			'last' => $last,
			'field' => $field,
			));
		$query->execute();
	}

	private function updateSearchConfig ($name, $mini, $full, $first, $last, $field = 0) {
		$table = 'visualscience_search_config';
		$query = db_update($table)->fields(array(
			'mini' => $mini,
			'full' => $full,
			'first' => $first,
			'last' => $last,
			'field' => $field,
			));
		
		$query->condition('name', $name);
		$query->execute();
	}

	private function getListOfFields () {
		$listFields = array();
		$userFields = user_load(0);
		return array_keys(get_object_vars($userFields));
	}

	private function getSelectedFields () {
		$query = db_select('visualscience_search_config', 'f')
		->fields('f', array('name', 'mini', 'full', 'first', 'last'));
		$result = $query->execute();
		$final = array();
		while ($record = $result->fetchAssoc()) {
			$final[$record['name']] = $record;
		}
		return $final;
	}

	private function saveFields () {
		$fieldsList = $this->getListOfFields();
		foreach ($fieldsList as $field) {
			if (isset($_POST['first']) && $_POST['first'] == $field) {
				if (isset($_POST['last']) && $_POST['last'] == $field) {
					$this->insertIntoSearchConfig($field, 1, 1, 1, 1, 0);
				}
				else {
					$this->insertIntoSearchConfig($field, 1, 1, 1, 0, 0);
				}
			}
			else if (isset($_POST['last']) && $_POST['last'] == $field) {
				$this->insertIntoSearchConfig($field, 1, 1, 0, 1, 0);
			}
			else if (isset($_POST[$field.'-mini']) && intval($_POST[$field.'-mini']) == 1) {
				$this->insertIntoSearchConfig($field, 1, 1, 0, 0, 0);
			}
			else if (isset($_POST[$field.'-full']) && intval($_POST[$field.'-full']) == 1){
				$this->insertIntoSearchConfig($field, 0, 1, 0, 0, 0);
			}
		}
	}

	public function getHtmlConfigPage () {
		$fieldsList = $this->getListOfFields();

		$oldFields = $this->getSelectedFields();
		$intro = $this->getIntroduction();
		$fieldsTable = $this->createFieldsTable($fieldsList, $oldFields);
		$saveButton = $this->createSaveButton();
		$formStart = '<form action="" method="POST" id="visualscience_config_form" >';
		$formEnd = '<input type="hidden" name="visualscience_config_form" /></form>';
		return $formStart.$intro.$fieldsTable.$saveButton.$formEnd;
	}

	public function saveSentValues () {
		$this->emptyOldValues();
		$this->saveFields();
	}

	public function insertPatternConfig ($field) {
		$this->insertIntoSearchConfig($field['name'], $field['mini'],$field['full'],$field['first'],$field['last']);
	}

	public function modifyPatternConfig ($field) {
		$this->updateSearchConfig($field['name'], $field['mini'],$field['full'],$field['first'],$field['last']);

	}

	public function checkCompleteField ($field) {
		if (!(isset($field['name']))) {
			return 'name';
		}
		if (!(isset($field['mini']))) {
			return 'mini';
		}
		if (!(isset($field['full']))) {
			return 'full';
		}
		if (!(isset($field['last']))) {
			return 'last';
		}
		if (!(isset($field['first']))) {
			return 'first';
		}
		return false;
	}

	public function fieldExistsInDB ($field) {
		$result = db_select('visualscience_search_config', 'c')
		->fields('c', array('name'))
		->condition('name', $field['name'])
		->range(0, 1)
		->execute()
		->rowCount();

		if ($result) {
			return true;
		}
		return false;
	}

	public function checkCorrectValueTypes ($field) {
		if (gettype($field['name']) != 'string') {
			return 'name';
		}
		if ($field['first'] != '0' && $field['first'] != '1') {
			return 'first';
		}
		if ($field['last'] != '0' && $field['last'] != '1') {
			return 'last';
		}
		if ($field['full'] != '0' && $field['full'] != '1') {
			return 'full';
		}
		if ($field['mini'] != '0' && $field['mini'] != '1') {
			return 'mini';
		}
		return false;
	}
}