<?php
class Search {

	private function ensureSearchSafety ($search) {
		//TODO: Implement it !
		return $search;
	}

	private function getFieldsFromConfig () {
		$query = db_select('visualscience_search_config', 'f')
		->fields('f', array('name','mini','full','first','last'));
		$result = $query->execute();
		$final = array();
		for ($i=0; $record = $result->fetchAssoc(); $i++) {
			$final[$record['name']] = $record;
		}
		return $final;
	}

	private function getValueOfField ($field, $user) {
		$value = $user->$field['name'];
		$ifDefField = field_view_field('user', $user, $field['name']);
		if (gettype($value) == 'object') {
			$value = 'Object';
		}
		if (gettype($value) == 'array' && !empty($ifDefField)) {
			$value = $ifDefField[0]['#markup'];
		}
		if (gettype($value) == 'array') {
			$list = '';
			foreach ($value as $innerVal) {
				if (gettype($innerVal) == 'array') {
					$list .= 'Array';
					break;
				}
				else {
					$list .= $innerVal . '; ';
				}
			}
			$value = $list;
		}
		return $value.'';
	}

	private function getUsersFields ($fields, $from=0, $to=0) {
		if ($to != 0) {
			$usersIds = array();
			while ($from <= $to) {
				array_push($usersIds, $from);
				$from++;
			}
		}
		else {
			$usersIds = $this->getAllUsersIds();
		}
		$users = user_load_multiple($usersIds);
		$userFields = array();
		foreach ($users as $user) {
			if ($user->name != '') { // check for anonymous user
				$userFields[$user->uid] = array();
				foreach ($fields as $field) {
					$valueOfField = $this->getValueOfField($field, $user);
					if ($field['first'] == 1) {
						$userFields[$user->uid]['first'] = $valueOfField;
					}
					else if ($field['last'] == 1) {
						$userFields[$user->uid]['last'] = $valueOfField;
					}
					else {
						$userFields[$user->uid][$field['name']] = $valueOfField;
					}
				}
			}
		}
		return $userFields;
	}

	private function getJsonUsersFields ($fields, $from, $to) {
		return json_encode($this->getUsersFields($fields, $from, $to));
	}

	private function getJsonDisplayConfig ($fields) {
		$config = '{"fields": [';
		$endConfig = ']';
		foreach ($fields as $field) {
			$config .= '{"name": "'.$field['name'].'","mini": '.$field['mini'].', "full": '.$field['full'].'},';
			if ($field['first'] == 1) {
				$endConfig .= ', "first": "'.$field['name'].'"';
			}
			if ($field['last'] == 1) {
				$endConfig .= ', "last": "'.$field['name'].'"';
			}
		}
		$config = substr($config, 0, strlen($config) -1) .$endConfig. '}';
		return $config;
	}

	private function getAllUsersIds () {
		$query = db_select('users', 'f')
		->fields('f', array('uid'));
		$result = $query->execute();
		$final = array();
		for ($i=0; $record = $result->fetchAssoc(); $i++) {
			array_push($final, $record['uid']);
		}
		return $final;
	}

	private function getCountOfUsers () {
		$query = db_select('users', 'f')
		->fields(NULL, array('uid'));
		$result = $query->execute()->fetchAll();
		return count($result);
	}

	public function getSavedSearch () {
		//TODO: Implement it
		if (isset($_GET['search'])) {
			return $_GET['search'];
		}
		return '';
	}

	public function getHtmlSearchBar ($searchValue= "") {
		$safeSearchVal = $this->ensureSearchSafety($searchValue);

		return '<div align="center">
		<input type="search" placeholder="Search..." val="'.$safeSearchVal.'" class="visualscience-search-main visualscience-search" id="visualscience-search-bar" onKeyUp="vsUserlist.search();" />
		<div style="width:98%;" align="left">
		<p class="visualscience-right" align="right" style="display:inline;max-width:30%;">'.l(t("Help"), "admin/help/visualscience").'</p>
		<p class="clickable" style="display:inline;max-width:30%;text-align:center;" align="center"><a onClick="vsUserlist.reloadUserDatabase(0);">Reload User Database</a></p>
		<p class="visualscience-left" align="right" style="visibility:hidden;display:inline;max-width:30%;text-align:center;margin-left:30%;"><a onClick="vsUserlist.saveSearch();">Save/Load</a></p>
		</div>
		</div>';
	}

	public function getHtmlSearchTable () {
		return '<div id="visualscience-container"></div>';
	}

	public function getJsonDatabase () {
		$fields = $this->getFieldsFromConfig();
		$jsonUsersAndFields = $this->getJsonUsersFields($fields);
		$jsonDisplayConfig = $this->getJsonDisplayConfig($fields);
		$searchDB = '{"users": '.$jsonUsersAndFields.', "config":'.$jsonDisplayConfig.'}';
		return '<script type="text/javascript" charset="utf-8">var vsSearchDB = '. $searchDB .';</script>';
		return $searchDB;
	}

	public function getClientSideFiles () {
		global $user;
		global $base_path;
		drupal_add_library('system', 'ui.autocomplete');
		drupal_add_library('system', 'ui.datepicker');
		drupal_add_library('system', 'ui.dialog');
		drupal_add_library('system', 'ui.tabs');
		drupal_add_library('system', 'ui.progressbar');

		drupal_add_js('http://livingscience.ethz.ch/livingscience/livingscience/livingscience.nocache.js', 'external');
		drupal_add_css(drupal_get_path('module', 'visualscience') .'/css/visualscience.css');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/lib/visualscience.jquery.layout.js');
		drupal_add_css(drupal_get_path('module', 'visualscience') .'/css/visualscience.jquery.layout.css');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/lib/visualscience.jquery.tablesorter.js');
		drupal_add_css(drupal_get_path('module', 'visualscience') .'/css/visualscience.jquery.tablesorter.css');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/lib/visualscience.handlebars.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/lib/visualscience.nddb.js');
  		//Settings necessary to VisualScience:
		drupal_add_js(array('installFolder' => $base_path.drupal_get_path('module', 'visualscience').'/'), 'setting');
		if (isset($user->name)) {
			drupal_add_js(array('username' => $user->name), 'setting');
		}
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.utils.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.database.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.interface.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.search.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.message.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.csv.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.userlist.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.lscomparison.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.search.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.conference.js');
		drupal_add_js(drupal_get_path('module', 'visualscience') .'/javascript/visualscience.livingscience.js');
	}

	public function getPatternConfiguration () {
		return $this->getFieldsFromConfig();
	}

	public function getUsersEntries ($from=0, $howMany=1000) {
		$final = $from + $howMany;
		$fields = $this->getFieldsFromConfig();
		$jsonUsersAndFields = $this->getJsonUsersFields($fields, $from, $final);
		$total = $this->getCountOfUsers();
		$jsonDisplayConfig = $this->getJsonDisplayConfig($fields);
		$searchDB = '{"users": '.$jsonUsersAndFields.', "config":'.$jsonDisplayConfig.', "from": '.$from.',  "howMany":'.$howMany.', "total": '.$total.'}';
		return $searchDB;
	}
}