<?php 
class Message {

	public function visualscience_send_message() {
		$subject =  $_POST['subject'];
		$email = $_POST['recipients']['email'];
		$name =  $_POST['recipients']['name'];
		$message =  $_POST['message'];
		$attachments=  $_POST['attachments'];// [0][0] will give the name of object n°0, while [0][1] will give its URL
		if ($attachments[0]) {
			$attachmentsText = t('<br /><h3>Attached Files</h3>');
			foreach ($attachments as $entry) {
				$attachmentsText .= t('- <a href="'.$entry[1].'" _target="blank">'.$entry[0].'</a><br />');
			}
		}
		else {
			$attachmentsText = '';
		}
		$final_text = $message.$attachmentsText;


		global $user;
		$module = 'VisualScience';
		$key = uniqid('mail');
		$language = language_default();
		$params = array();
		$from = $user->mail;
		$send = FALSE;
		$message = drupal_mail($module, $key, $email, $language, $params, $from, $send);

		$message['headers']['Content-Type'] = 'text/html; charset=UTF-8; format=flowed';
		$message['headers']['From'] = $message['headers']['Sender'] = $message['headers']['Return-Path'] = $from;
		$message['subject'] = $subject;
		$message['body'] = $final_text;

    // Retrieve the responsible implementation for this message.
		$system = drupal_mail_system($module, $key);

    // Send e-mail.
		$message['result'] = $system->mail($message);
	//If no errors, let's add file access to the user.
		if ($message['result'] == 1) {
		//getting user id from email.
			$users = db_query('SELECT uid FROM {users} WHERE mail = :mail LIMIT 1', array(':mail' => $email,));
		//In the "impossible" case where two emails are the same in the db
			$users = $users->fetchObject();
			$uid = $users->uid;
		//actually adding the access to the user.
			if (!is_null($uid) && isset($uid)) {	
				foreach ($attachments as $file) {
					$query = db_insert('visualscience_uploaded_files')->fields(array(
						'uid' => $uid,
						'email' => $from,
						'name' => $file[0],
						'url' => $file[1],
						));
					$query->execute();
				}
			}
			echo '1';
		}
		else {
			echo '0';
		}
	}
}