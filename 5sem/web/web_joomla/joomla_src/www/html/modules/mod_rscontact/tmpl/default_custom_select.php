<?php
/**
* @package RSContact!
* @copyright (C) 2015 www.rsjoomla.com
* @license GPL, http://www.gnu.org/licenses/gpl-3.0.html
*/

defined('_JEXEC') or die('Restricted access');

use Joomla\CMS\Language\Text;
use Joomla\CMS\HTML\HTMLHelper;

 if (${"cf" .$i. "_input"}) {
		if(is_string(${"cf" .$i. "_input"})) {
			$options = modRSContactHelper::split(${"cf" .$i. "_input"});
		}
		if(!$show_label ) {
			if(${"show_cf" .$i. "_req"}) {
				$cf_select = Text::_('MOD_RSCONTACT_CUSTOM_FIELD'.$i.'_SELECT').' '.${"cf" . $i. "_name"}.' '.$required_marker;
			} else {
				$cf_select = Text::_('MOD_RSCONTACT_CUSTOM_FIELD'.$i.'_SELECT').' '.${"cf" . $i. "_name"};
			}
		} else {
			$cf_select = Text::_('MOD_RSCONTACT_CUSTOM_FIELD'.$i.'_SELECT');
		}
		
		${"data_cf" .$i }[] = HTMLHelper::_('select.option', "", $cf_select);
		foreach ($options as $input) {
			${"data_cf" .$i }[] = HTMLHelper::_('select.option', $input, $input);
		}
		echo HTMLHelper::_('select.genericlist', ${"data_cf" .$i }, 'mod_rscontact_cf'.$i, 'class="'.$class.'"', 'value', 'text', null, 'mod_rscontact_cf'.$i.'_'.$uniqid);
		
	 } else { ?>
		<div class="alert alert-error alert-danger">
			<?php echo Text::sprintf('MOD_RSCONTACT_INPUT_ERROR', ${"cf" . $i. "_name"}); ?>
		</div>
<?php } ?>