<?php
/**
 * @package RSContact!
 * @copyright (C) 2015 www.rsjoomla.com
 * @license GPL, http://www.gnu.org/licenses/gpl-3.0.html
 */

defined('_JEXEC') or die('Restricted access');

use Joomla\CMS\Language\Text;
use Joomla\CMS\HTML\HTMLHelper;
use Joomla\CMS\Factory;
use Joomla\CMS\Captcha\Captcha;
use Joomla\CMS\Date\Date;
use Joomla\CMS\Helper\ModuleHelper;
use Joomla\Utilities\IpHelper;
use Joomla\Registry\Registry;
use Joomla\CMS\Uri\Uri;
use Joomla\CMS\Mail\MailHelper;
use Joomla\CMS\Session\Session;

Text::script('MOD_RSCONTACT_SALUTATION_ERROR');
Text::script('MOD_RSCONTACT_FIRST_NAME_ERROR');
Text::script('MOD_RSCONTACT_LAST_NAME_ERROR');
Text::script('MOD_RSCONTACT_FULL_NAME_ERROR');
Text::script('MOD_RSCONTACT_EMAIL_ERROR');
Text::script('MOD_RSCONTACT_ADDRESS_1_ERROR');
Text::script('MOD_RSCONTACT_ADDRESS_2_ERROR');
Text::script('MOD_RSCONTACT_CITY_ERROR');
Text::script('MOD_RSCONTACT_STATE_ERROR');
Text::script('MOD_RSCONTACT_ZIP_ERROR');
Text::script('MOD_RSCONTACT_ZIP_NOT_A_ALPHANUMERIC_ERROR');
Text::script('MOD_RSCONTACT_HOME_PHONE_ERROR');
Text::script('MOD_RSCONTACT_MOBILE_PHONE_ERROR');
Text::script('MOD_RSCONTACT_WORK_PHONE_ERROR');
Text::script('MOD_RSCONTACT_PHONE_NOT_A_NUMBER_ERROR');
Text::script('MOD_RSCONTACT_COMPANY_ERROR');
Text::script('MOD_RSCONTACT_WEBSITE_ERROR');
Text::script('MOD_RSCONTACT_SUBJECT_ERROR');
Text::script('MOD_RSCONTACT_MESSAGE_ERROR');
Text::script('MOD_RSCONTACT_CHARACTERS_LEFT');

class modRSContactHelper
{
	static $states = array(
		'AK' => 'Alaska',
		'AL' => 'Alabama',
		'AR' => 'Arkansas',
		'AZ' => 'Arizona',
		'CA' => 'California',
		'CO' => 'Colorado',
		'CT' => 'Connecticut',
		'DC' => 'District of Columbia',
		'DE' => 'Delaware',
		'FL' => 'Florida',
		'GA' => 'Georgia',
		'HI' => 'Hawaii',
		'IA' => 'Iowa',
		'ID' => 'Idaho',
		'IL' => 'Illinois',
		'IN' => 'Indiana',
		'KS' => 'Kansas',
		'KY' => 'Kentucky',
		'LA' => 'Louisiana',
		'MA' => 'Massachusetts',
		'MD' => 'Maryland',
		'ME' => 'Maine',
		'MI' => 'Michigan',
		'MN' => 'Minnesota',
		'MO' => 'Missouri',
		'MS' => 'Mississippi',
		'MT' => 'Montana',
		'NC' => 'North Carolina',
		'ND' => 'North Dakota',
		'NE' => 'Nebraska',
		'NH' => 'New Hampshire',
		'NJ' => 'New Jersey',
		'NM' => 'New Mexico',
		'NV' => 'Nevada',
		'NY' => 'New York',
		'OH' => 'Ohio',
		'OK' => 'Oklahoma',
		'OR' => 'Oregon',
		'PA' => 'Pennsylvania',
		'PR' => 'Puerto Rico',
		'RI' => 'Rhode Island',
		'SC' => 'South Carolina',
		'SD' => 'South Dakota',
		'TN' => 'Tennessee',
		'TX' => 'Texas',
		'UT' => 'Utah',
		'VA' => 'Virginia',
		'VT' => 'Vermont',
		'WA' => 'Washington',
		'WI' => 'Wisconsin',
		'WV' => 'West Virginia',
		'WY' => 'Wyoming',
		'OU' => 'Outside US'
	);

	public static function loadJs($file)
	{
		HTMLHelper::_('script', 'mod_rscontact/'.$file.'.js', array('relative' => true, 'version' => 'auto'));
	}

	public static function loadCss($file)
	{
		HTMLHelper::_('stylesheet', 'mod_rscontact/'.$file.'.css', array('relative' => true, 'version' => 'auto'));
	}

	protected static function encodeHTML(&$item, $key)
	{
		$item = self::cleanInput($item);
	}

	protected static function flatten(&$item, $key)
	{
		if (is_array($item))
		{
			$item = implode(', ', $item);
		}
	}

	public static function cleanInput($input)
	{
		if ($input === null)
		{
			$input = '';
		}
		elseif (is_array($input))
		{
			$input = implode(', ', $input);
		}

		return htmlspecialchars($input, ENT_QUOTES, "UTF-8");
	}

	protected static function replacePlaceholders($text, $placeholders, $escapeCallable = null)
	{
		// Performance check
		if (strpos($text, '{') === false)
		{
			return $text;
		}

		array_walk($placeholders, array('modRSContactHelper', 'flatten'));

		// Escape placeholders with user supplied function
		if ($escapeCallable)
		{
			// Built-in "HTML" encoding function
			if (strtolower($escapeCallable) == 'html')
			{
				$escapeCallable = array('modRSContactHelper', 'encodeHTML');
			}

			if (is_callable($escapeCallable))
			{
				array_walk($placeholders, $escapeCallable);
			}
		}

		return str_replace(array_keys($placeholders), array_values($placeholders), $text);
	}

	public static function captchaGenerate($event, $value = null, $id = '')
	{
		if ($value !== null && !strlen(trim($value)))
		{
			$value = null;
		}

		try
		{
			$captcha = Captcha::getInstance(Factory::getConfig()->get('captcha'));
		}
		catch (Exception $e)
		{
			Factory::getApplication()->enqueueMessage($e->getMessage(), 'error');
			return false;
		}

		if (!$captcha)
		{
			Factory::getApplication()->enqueueMessage(Text::_('MOD_RSCONTACT_NO_CAPTCHA_CONFIGURED'), 'error');
			return false;
		}

		switch ($event)
		{
			case 'onDisplay':
				return $captcha->display('mod-rscontact-captcha-' . $id, 'mod-rscontact-captcha-' . $id, 'required');
				break;

			case 'onCheckAnswer':
				return $captcha->checkAnswer($value);
				break;

			default:
				return false;
				break;
		}
	}

	public static function split($input)
	{
		$options = trim($input);
		$options = str_replace(array("\r\n", "\r"), "\n", $options);
		$options = preg_split("/[\n,]+/", $options);
		return $options;
	}

	protected static function showResponse($status, $message, $warnings = array())
	{
		$response = (object) array(
			'status' 	=> $status,
			'message' 	=> $message,
			'warnings' 	=> $warnings
		);
		Factory::getDocument()->setMimeEncoding('application/json');

		echo json_encode($response);

		Factory::getApplication()->close();
	}

	public static function isEmailAddress($value)
	{
		return MailHelper::isEmailAddress($value);
	}

	public static function getAjax(){
		Factory::getLanguage()->load('mod_rscontact');
		$warning 	= array();
		$jInput		= Factory::getApplication()->input;
		//ajax submit
		$inputs 	= $jInput->get('data', array(), 'array');

		$user 		= Factory::getUser();
		$config		= Factory::getConfig();

		$user_id 	= $user->get('id');
		$username	= $user->get('username');
		$user_email = $user->get('email');

		$timeZone	= $config->get('offset');
		$myDate 	= Date::getInstance('now', $timeZone);
		$date 		= $myDate->format('d-m-Y', true, true);
		$date_time	= $myDate->format('d-m-Y H:i:s', true, true);

		if (!empty($inputs['mod_rscontact_module_id']))
		{
			$module = ModuleHelper::getModuleById($inputs['mod_rscontact_module_id']);
		}
		else
		{
			$module = ModuleHelper::getModule('rscontact', $inputs['mod_rscontact_module_name']);
		}
		$params = new Registry();
		$params->loadString($module->params);

		$recipient		= $params->get('mail_to', '');
		$bcc 			= $params->get('mail_bcc', '');
		$cc 			= $params->get('mail_cc', '');
		$message_set	= $params->get('mail_msg', '');
		$fullname_email	= $params->get('name_type') == 1;
		$thank_you		= $params->get('thank_you', Text::_('MOD_RSCONTACT_THANK_YOU_DEFAULT'));
		$send_copy		= $params->get('send_copy') == 1;
		$show_captcha	= $params->get('captcha');
		$show_consent   = $params->get('display_consent');
		$subject_predef	= $params->get('email_subj', '');
		$set_reply		= $params->get('reply_to', '');
		$reply_email	= $params->get('reply_email', '');
		$reply_name	    = $params->get('reply_name', '');
		$ip_remote		= IpHelper::getIp();

		$salut_form 	= !empty($inputs['mod_rscontact_salutation'])   	? $inputs['mod_rscontact_salutation']	: '';
		$first_name		= !empty($inputs['mod_rscontact_first_name'])   	? $inputs['mod_rscontact_first_name']	: '';
		$last_name		= !empty($inputs['mod_rscontact_last_name'])    	? $inputs['mod_rscontact_last_name']	: '';
		$fullname		= !empty($inputs['mod_rscontact_full_name'])    	? $inputs['mod_rscontact_full_name']	: '';
		$email			= !empty($inputs['mod_rscontact_email'])	    	? $inputs['mod_rscontact_email'] 		: '';
		$address_1		= !empty($inputs['mod_rscontact_address_1'])    	? $inputs['mod_rscontact_address_1']	: '';
		$address_2		= !empty($inputs['mod_rscontact_address_2'])    	? $inputs['mod_rscontact_address_2']	: '';
		$city			= !empty($inputs['mod_rscontact_city'])		    	? $inputs['mod_rscontact_city']			: '';
		$state			= !empty($inputs['mod_rscontact_states'])	    	? $inputs['mod_rscontact_states']		: '';
		$zip			= !empty($inputs['mod_rscontact_zip'])			    ? $inputs['mod_rscontact_zip']			: '';
		$h_phone 		= !empty($inputs['mod_rscontact_home_phone'])	    ? $inputs['mod_rscontact_home_phone']	: '';
		$m_phone		= !empty($inputs['mod_rscontact_mobile_phone'])     ? $inputs['mod_rscontact_mobile_phone']	: '';
		$w_phone		= !empty($inputs['mod_rscontact_work_phone'])	    ? $inputs['mod_rscontact_work_phone']	: '';
		$company		= !empty($inputs['mod_rscontact_company'])	    	? $inputs['mod_rscontact_company']		: '';
		$website		= !empty($inputs['mod_rscontact_website'])	    	? $inputs['mod_rscontact_website']		: '';
		$subject		= !empty($inputs['mod_rscontact_subject'])	    	? $inputs['mod_rscontact_subject']		: '';
		$message		= !empty($inputs['mod_rscontact_message'])	    	? $inputs['mod_rscontact_message']		: '';
		$cf1			= !empty($inputs['mod_rscontact_cf1'])			    ? $inputs['mod_rscontact_cf1']			: '';
		$cf2			= !empty($inputs['mod_rscontact_cf2'])			    ? $inputs['mod_rscontact_cf2']			: '';
		$cf3			= !empty($inputs['mod_rscontact_cf3'])			    ? $inputs['mod_rscontact_cf3']			: '';
		$selfcopy		= !empty($inputs['mod_rscontact_selfcopy'])		    ? $inputs['mod_rscontact_selfcopy']		: '';
		$consent		= !empty($inputs['mod_rscontact_display_consent'])	? $inputs['mod_rscontact_display_consent'] : '';

		try
		{
			if (!Session::checkToken()) {
				throw new Exception(Text::_('MOD_RSCONTACT_INVALID_TOKEN'));
			}

			if(!static::isEmailAddress($email)){
				throw new Exception(Text::_('MOD_RSCONTACT_EMAIL_ERROR'));
			}

			if(!$recipient){
				throw new Exception(Text::_('MOD_RSCONTACT_EMAIL_TO_ERROR'));
			}

			if($show_consent && !$consent) {
				throw new Exception(Text::_('MOD_RSCONTACT_DISPLAY_CONSENT_ERROR'));
			}

			if ($show_captcha && !self::captchaGenerate('onCheckAnswer', null, $module->id)) {
				throw new Exception(Text::_('MOD_RSCONTACT_CAPTCHA_ERROR'));
			}

			if($fullname_email){
				$sender = $fullname;
			}
			else {
				$sender = $first_name.' '.$last_name;
			}

			if (isset(self::$states[$state])) {
				$state = self::$states[$state];
			}

			$placeholders = array(
				'{salut-form}'			=> $salut_form,
				'{salutation}'			=> $salut_form,
				'{first-name}'			=> $first_name,
				'{last-name}'			=> $last_name,
				'{fullname}'			=> $fullname,
				'{subject}'				=> $subject,
				'{email}'				=> $email,
				'{address-1}'			=> $address_1,
				'{address-2}'			=> $address_2,
				'{city}'				=> $city,
				'{state}'				=> $state,
				'{zip}'					=> $zip,
				'{home-phone}'			=> $h_phone,
				'{mobile-phone}'		=> $m_phone,
				'{work-phone}'			=> $w_phone,
				'{company}'				=> $company,
				'{website}'				=> $website,
				'{message}'				=> $message,
				'{cf1}'					=> $cf1,
				'{cf2}'					=> $cf2,
				'{cf3}'					=> $cf3,
				'{consent}'             => Text::_('MOD_RSCONTACT_DISPLAY_CONSENT_PLACEHOLDER'),
				'{username}'			=> $username,
				'{user-id}'				=> $user_id,
				'{user-email}'			=> $user_email,
				'{date}'				=> $date,
				'{date-time}'			=> $date_time,
				'{ip}'					=> $ip_remote,
				'{your-website}'		=> $config->get('sitename'),
				'{your-website-url}' 	=> Uri::root()
			);

			// Replace placeholders for the email body
			$msg = self::replacePlaceholders($message_set, $placeholders, 'html');

			// Replace placeholders for the email subject
			$subject_predef	= self::replacePlaceholders($subject_predef, $placeholders);

			// Replace placeholders for the Thank You message
			$thank_you = self::replacePlaceholders($thank_you, $placeholders, 'html');

			// array email addresses
			$recipient	= array_filter(preg_split('/[;,]+/', $recipient), array('modRSContactHelper', 'isEmailAddress'));
			$bcc		= array_filter(preg_split('/[;,]+/', $bcc), array('modRSContactHelper', 'isEmailAddress'));
			$cc			= array_filter(preg_split('/[;,]+/', $cc), array('modRSContactHelper', 'isEmailAddress'));

			if (!$set_reply)
			{
				$replyTo = self::replacePlaceholders($reply_email, $placeholders);
				$replyToName = self::replacePlaceholders($reply_name, $placeholders);
			}
			else
			{
				$replyTo = $email;
				$replyToName = $sender;
			}

			if (!$replyTo || !static::isEmailAddress($replyTo))
			{
				$replyTo = null;
				$replyToName = null;
			}

			// send admin email
			$sent_admin = Factory::getMailer()->sendMail($config->get('mailfrom'), $sender, $recipient, $subject_predef, $msg, true, $cc, $bcc, null, $replyTo, $replyToName);

			// send selfcopy email
			if ($selfcopy || $send_copy) {
				$subject = Text::sprintf('MOD_RSCONTACT_SEND_COPY_SUBJECT', $config->get('sitename'));

				$sent_user = Factory::getMailer()->sendMail($config->get('mailfrom'), $config->get('fromname'), $email, $subject, $msg, true);
				if ($sent_user !== true) {
					$errorMessage = Text::_('MOD_RSCONTACT_NO_FURTHER_INFORMATION_AVAILABLE');
					if (is_object($sent_user) && is_callable(array($sent_user, 'getMessage'))) {
						$errorMessage = $sent_user->getMessage();
					}
					$warning[] = Text::sprintf('MOD_RSCONTACT_EMAIL_FAILED_COPY', $errorMessage);
				}
			}

			if ($sent_admin !== true) {
				$db = Factory::getDbo();
				$jdate = new Date('now');
				$query = $db->getQuery(true);

				// Get all admin users for database
				$query->clear()
					->select($db->qn(array('id', 'name', 'email', 'sendEmail')))
					->from($db->qn('#__users'))
					->where($db->qn('sendEmail') . ' = ' . 1);

				$db->setQuery($query);
				if ($rows = $db->loadObjectList()) {
					foreach ($rows as $row) {
						$user_send_from = $user_id ? $user_id : $row->id;
						$not_sent 		= Text::sprintf('MOD_RSCONTACT_ADMIN_EMAIL_NOT_SENT', '<strong>'.$params->get('mail_to').'</strong><br />');
						$values = array($db->q($user_send_from), $db->q($row->id), $db->q($jdate->toSql()), $db->q($subject_predef), $db->q($not_sent.$msg));
						$query->clear()
							->insert($db->qn('#__messages'))
							->columns($db->qn(array('user_id_from', 'user_id_to', 'date_time', 'subject', 'message')))
							->values(implode(',', $values));
						$db->setQuery($query);
						$db->execute();
					}
				}

				$errorMessage = Text::_('MOD_RSCONTACT_NO_FURTHER_INFORMATION_AVAILABLE');
				if (is_object($sent_admin) && is_callable(array($sent_admin, 'getMessage'))) {
					$errorMessage = $sent_admin->getMessage();
				}

				$warning[] = Text::sprintf('MOD_RSCONTACT_EMAIL_FAILED', $errorMessage);
			}

			self::showResponse(1, $thank_you, $warning);
		} catch (Exception $e) {
			self::showResponse(0, $e->getMessage());
		}
	}
}