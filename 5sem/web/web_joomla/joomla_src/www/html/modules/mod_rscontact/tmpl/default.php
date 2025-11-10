<?php
/**
* @package RSContact!
* @copyright (C) 2015 www.rsjoomla.com
* @license GPL, http://www.gnu.org/licenses/gpl-3.0.html
*/

defined('_JEXEC') or die('Restricted access');

use Joomla\CMS\HTML\HTMLHelper;
use Joomla\CMS\Language\Text;
use Joomla\CMS\Uri\Uri;
use Joomla\CMS\Factory;
use Joomla\CMS\Filter\OutputFilter;
use Joomla\CMS\Helper\ModuleHelper;
?>
<?php $class_auto = $auto_width ? ' mod-rscontact-width-auto' : ""; ?>
<div id="mod-rscontact-container-<?php echo $uniqid;?>" class="rscontact">
    <form id="mod-rscontact-contact-form-<?php echo $uniqid; ?>" name="mod-rscontact-contact-form" enctype="multipart/form-data" method="post" action="<?php echo modRSContactHelper::cleanInput(Uri::current()); ?>" <?php if($form_horizontal) echo 'class="form-horizontal"';?>>
		<fieldset>
		<?php echo HTMLHelper::_( 'form.token' ); ?>
		<input type="hidden" name="mod_rscontact_module_id" value="<?php echo $module->id; ?>" />
		<input type="hidden" name="mod_rscontact_module_name" value="<?php echo modRSContactHelper::cleanInput($module->title); ?>" />
		<?php if ($form_pre_text) { ?>
			<div class="mod-rscontact-pre-text">
				<?php echo $form_pre_text; ?>
			</div>
		<?php } ?>

		<?php if ($show_salutation) {
				//salut array forms
				if ($show_salutation_req && !$show_label) {
					$salutation_select = Text::_('MOD_RSCONTACT_SALUTATION_SELECT').' '.$required_marker;
				}else{
					$salutation_select = Text::_('MOD_RSCONTACT_SALUTATION_SELECT');
				}

				$forms = array(
					HTMLHelper::_('select.option', "", $salutation_select),
					HTMLHelper::_('select.option', Text::_('MOD_RSCONTACT_SALUTATION_MR')),
					HTMLHelper::_('select.option', Text::_('MOD_RSCONTACT_SALUTATION_MS')),
					HTMLHelper::_('select.option', Text::_('MOD_RSCONTACT_SALUTATION_MRS')),
					HTMLHelper::_('select.option', Text::_('MOD_RSCONTACT_SALUTATION_DR')),
					HTMLHelper::_('select.option', Text::_('MOD_RSCONTACT_SALUTATION_PROF'))
				);

				if ($show_salutation_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				} ?>

			<div class="control-group">
			<?php if ($show_label) { ?>
				<label for="mod_rscontact_salutation" class="control-label">
					<?php echo Text::_('MOD_RSCONTACT_SALUTATION');
						  if ($show_salutation_req) { ?>
						      <span class="help-inline"><?php echo $required_marker; ?></span>
					<?php } ?>
				</label>
			<?php } ?>
				<div class="controls">
					<?php echo HTMLHelper::_('select.genericlist', $forms, 'mod_rscontact_salutation', 'class="'.$class.'"', 'value', 'text', null, 'mod_rscontact_salutation_'.$uniqid); ?>
				</div>
			</div>
		<?php } ?>

		<?php if ($show_name) {
				if ($show_name_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				} ?>
		<?php if ($show_full_name) {
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_FULL_NAME').'"' : '';
				if($show_name_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_FULL_NAME').' '.$required_marker.'"';
				}
			    ?>
				<div class="control-group">
				<?php if ($show_label) { ?>
					<label for="mod-rscontact-full-name-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_FULL_NAME');
							  if ($show_name_req) { ?>
								<span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
				<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_full_name" id="mod-rscontact-full-name-<?php echo $uniqid; ?>" type="text" value="" class="<?php echo $class; ?>" <?php	echo $placeholder; ?> />
					</div>
				</div>
		<?php } else {
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_FIRST_NAME').'"' : '';
				if($show_name_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_FIRST_NAME').' '.$required_marker.'"';
				}
				?>
				<div class="control-group">
				<?php if ($show_label) { ?>
					<label for="mod-rscontact-first-name-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_FIRST_NAME');
							  if ($show_name_req) { ?>
								<span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
				<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_first_name" id="mod-rscontact-first-name-<?php echo $uniqid; ?>" type="text" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>

				<?php $placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_LAST_NAME').'"' : '';
					  if($show_name_req && !$show_label){
						$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_LAST_NAME').' '.$required_marker.'"';
					  }
				?>
				<div class="control-group">
				<?php if ($show_label) { ?>
					<label for="mod-rscontact-last-name-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_LAST_NAME');
							  if ($show_name_req) { ?>
								<span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
				<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_last_name" id="mod-rscontact-last-name-<?php echo $uniqid; ?>" type="text" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		  <?php } ?>
		<?php } ?>

		<?php $placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_EMAIL').' '.$required_marker.'"' : ''; ?>
		<div class="control-group">
		<?php if ($show_label) {  ?>
			<label for="mod-rscontact-email-<?php echo $uniqid; ?>" class="control-label">
				<?php echo Text::_('MOD_RSCONTACT_EMAIL').'<span class="help-inline">'.$required_marker.'</span>'; ?>
			</label>
		<?php } ?>
			<div class="controls">
				<input name="mod_rscontact_email" id="mod-rscontact-email-<?php echo $uniqid; ?>" type="email" class="required <?php echo $class_auto;?>" value="<?php echo Factory::getUser()->email; ?>" <?php echo $placeholder; ?>  />
			</div>
		</div>

		<?php if ($show_address_1) { ?>
			<?php if ($address_1_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_ADDRESS_1').'"' : '';
				if($address_1_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_ADDRESS_1').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
				<?php if ($show_label) { ?>
					<label for="mod-rscontact-address-1-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_ADDRESS_1');
							 if ($address_1_req) { ?>
							     <span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
				<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_address_1" id="mod-rscontact-address-1-<?php echo $uniqid; ?>" type="text" value="" class="<?php echo $class;?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		<?php } ?>

		<?php if ($show_address_2) { ?>
			<?php if ($address_2_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_ADDRESS_2').'"' : '';
				if($address_2_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_ADDRESS_2').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
				<?php if ($show_label) { ?>
					<label for="mod-rscontact-address-2-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_ADDRESS_2');
							  if ($address_2_req) { ?>
							     <span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
				<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_address_2" id="mod-rscontact-address-2-<?php echo $uniqid; ?>" type="text" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		<?php } ?>

		<?php if ($show_city) { ?>
			<?php if ($show_city_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_CITY').'"' : '';
				if($show_city_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_CITY').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
			<?php if ($show_label) { ?>
					<label for="mod-rscontact-city<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_CITY');
							  if ($show_city_req) { ?>
								<span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
			<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_city" id="mod-rscontact-city<?php echo $uniqid; ?>" type="text" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		<?php } ?>

		<?php if ($show_state) { ?>
			<?php if ($show_state_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}

				if ($show_state_req && !$show_label) {
					$state_select = Text::_('MOD_RSCONTACT_STATES_SELECT').' '.$required_marker;
				} else {
					$state_select = Text::_('MOD_RSCONTACT_STATES_SELECT');
				}

				$data_state[] = HTMLHelper::_('select.option', "", $state_select);
				foreach (modRSContactHelper::$states as $stateCode => $stateText) {
					$data_state[] = HTMLHelper::_('select.option', $stateCode, $stateText);
				} ?>
				<div class="control-group">
			<?php if ($show_label) { ?>
					<label for="mod_rscontact_states" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_STATES');
							  if ($show_state_req) { ?>
								<span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
			<?php } ?>
					<div class="controls">
					<?php
						echo HTMLHelper::_('select.genericlist', $data_state, 'mod_rscontact_states', 'class="'.$class.'"', 'value', 'text', null, 'mod_rscontact_states_'.$uniqid);
					?>
					</div>
				</div>
		<?php } ?>

		<?php if ($show_zip) { ?>
			<?php if ($show_zip_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_ZIP').'"' : '';
				if($show_zip_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_ZIP').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
			<?php if ($show_label) { ?>
					<label for="mod-rscontact-zip-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_ZIP');
							  if ($show_zip_req) { ?>
							    <span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
			<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_zip" id="mod-rscontact-zip-<?php echo $uniqid; ?>" type="text" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		<?php } ?>

		<?php if ($show_home_phone) { ?>
			<?php if ($home_phone_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_HOME_PHONE').'"' : '';
				if($home_phone_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_HOME_PHONE').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
			<?php if ($show_label) { ?>
					<label for="mod-rscontact-home-phone-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_HOME_PHONE');
							  if ($home_phone_req) { ?>
						         <span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
			<?php } ?>
					<div class="controls">
					  <input name="mod_rscontact_home_phone" id="mod-rscontact-home-phone-<?php echo $uniqid; ?>" type="tel" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		<?php } ?>

		<?php if ($show_mobile_phone) { ?>
			<?php if ($mobile_phone_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_MOBILE_PHONE').'"' : '';
				if($mobile_phone_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_MOBILE_PHONE').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
			<?php if ($show_label) { ?>
					<label for="mod-rscontact-mobile-phone-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_MOBILE_PHONE');
							  if ($mobile_phone_req) { ?>
								<span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
			<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_mobile_phone" id="mod-rscontact-mobile-phone-<?php echo $uniqid; ?>" type="tel" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		<?php } ?>

		<?php if ($show_work_phone) { ?>
			<?php if ($work_phone_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_WORK_PHONE').'"' : '';
				if($work_phone_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_WORK_PHONE').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
			<?php if ($show_label) { ?>
					<label for="mod-rscontact-work-phone-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_WORK_PHONE');
							  if ($work_phone_req) { ?>
							    <span class="help-inline"><?php echo $required_marker;?></span>
							<?php } ?>
					</label>
			<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_work_phone" id="mod-rscontact-work-phone-<?php echo $uniqid; ?>" type="tel" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		<?php } ?>

		<?php if ($show_company) { ?>
			<?php if ($company_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_COMPANY').'"' : '';
				if($company_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_COMPANY').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
			<?php if ($show_label) { ?>
					<label for="mod-rscontact-company-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_COMPANY');
							  if ($company_req) { ?>
								<span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
			<?php } ?>
					<div class="controls">
					  <input name="mod_rscontact_company" id="mod-rscontact-company-<?php echo $uniqid; ?>" type="text" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		<?php } ?>

		<?php if ($show_website) { ?>
			<?php if ($website_req) {
					$class = 'required'.$class_auto;
				  } else {
					$class = 'ignore'.$class_auto;
				  }
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_WEBSITE').'"' : '';
				if($website_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_WEBSITE').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
			<?php if ($show_label) { ?>
					<label for="mod-rscontact-website-<?php echo $uniqid; ?>" class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_WEBSITE');
							  if ($website_req) { ?>
						        <span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
			<?php } ?>
					<div class="controls">
						<input name="mod_rscontact_website" id="mod-rscontact-website-<?php echo $uniqid; ?>" type="url" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
					</div>
				</div>
		<?php } ?>

		<?php if ($show_subject) { ?>
			<?php if ($subject_req) {
					$class = 'required'.$class_auto;
				  } else {
					$class = 'ignore'.$class_auto;
				  }
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_SUBJECT').'"' : '';
				if($subject_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_SUBJECT').' '.$required_marker.'"';
				}
			?>
				<div class="control-group">
			<?php if ($show_label) {?>
					<label for="mod-rscontact-subject-<?php echo $uniqid; ?>"  class="control-label">
						<?php echo Text::_('MOD_RSCONTACT_SUBJECT');
							  if ($subject_req) { ?>
								<span class="help-inline"><?php echo $required_marker;?></span>
						<?php } ?>
					</label>
			<?php } ?>
					<div class="controls">
			<?php switch ($subject_type) {
					case "input": ?>
						<input type="text" name="mod_rscontact_subject" id="mod-rscontact-subject-<?php echo $uniqid; ?>" value="" class="<?php echo $class; ?>" <?php echo $placeholder; ?> />
				<?php break; ?>

			<?php	case "textarea": ?>
						<textarea rows="5" name="mod_rscontact_subject" id="mod-rscontact-subject-<?php echo $uniqid; ?>" class="<?php echo $class; ?>" <?php echo $placeholder; ?> ></textarea>
				<?php  break; ?>

			<?php	case "select":
						if ($subject_input) {
							if (is_string($subject_input)) {
								$options = modRSContactHelper::split($subject_input);
							}

							if ($subject_req && !$show_label){
								$subject_select = Text::_('MOD_RSCONTACT_SUBJECT_SELECT').' '.$required_marker;
							} else {
								$subject_select = Text::_('MOD_RSCONTACT_SUBJECT_SELECT');
							}

							$data_subject[] = HTMLHelper::_('select.option', "", $subject_select);
							foreach ($options as $input) {
								$data_subject[] = HTMLHelper::_('select.option', $input, $input);
							}
							echo HTMLHelper::_('select.genericlist', $data_subject, 'mod_rscontact_subject', array('class' => $class), 'value', 'text', null, 'mod-rscontact-subject-'.$uniqid);

						} else { ?>
							<div class="alert alert-error alert-danger">
								<?php Text::sprintf('MOD_RSCONTACT_INPUT_ERROR', Text::_('MOD_RSCONTACT_SUBJECT')); ?>
							</div>
					<?php }
						break; ?>

			<?php	case "radio":
						if ($subject_input) {
							if (is_string($subject_input)) {
								$options = modRSContactHelper::split($subject_input);
							}

						if (!$show_label) { ?>
							<label for="mod_rscontact_subject"  class="control-label">
								<?php echo Text::_('MOD_RSCONTACT_SUBJECT');?>
								<?php if ($subject_req) { ?>
										<span class="help-inline"><?php echo $required_marker;?></span>
								<?php } ?>
							</label>
					<?php }
						foreach ($options as $option) {
							$id = 'mod-rscontact-subject-'.OutputFilter::stringURLSafe($option).'-'.$uniqid; ?>
							<label for="<?php echo $id;?>" class="radio">
								<input type="radio" class="<?php echo $class; ?>" id="<?php echo $id;?>" name="<?php echo 'mod_rscontact_subject'; ?>" value="<?php echo $option; ?>" /><?php echo $option; ?>
							</label>
					<?php } ?>
							<label for="mod_rscontact_subject" class="mod-rscontact-error" style="display: none;"></label>
					<?php } else { ?>
							 <div class="alert alert-error alert-danger">
								<?php echo Text::sprintf('MOD_RSCONTACT_INPUT_ERROR', Text::_('MOD_RSCONTACT_SUBJECT')); ?>
							 </div>
					<?php }
						break; ?>

			<?php	case "checkbox":
						if ($subject_input) {
							if (is_string($subject_input)) {
								$options = modRSContactHelper::split($subject_input);
							}

						if (!$show_label) { ?>
							<label for="mod_rscontact_subject"  class="control-label">
								<?php echo Text::_('MOD_RSCONTACT_SUBJECT');?>
								<?php if ($subject_req) { ?>
									   <span class="help-inline"><?php echo $required_marker;?></span>
								<?php } ?>
							</label>
					<?php }
						foreach ($options as $option) {
							$id = 'mod-rscontact-subject-'.OutputFilter::stringURLSafe($option).'-'.$uniqid; ?>
							<label for="<?php echo $id;?>" class="checkbox">
								<input type="checkbox" class="<?php echo $class; ?>" id="<?php echo $id;?>" name="<?php echo 'mod_rscontact_subject'; ?>[]" value="<?php echo $option; ?>" /><?php echo $option; ?>
							</label>
					<?php } ?>
							<label for="mod_rscontact_subject" class="mod-rscontact-error" style="display: none;"></label>
					<?php } else { ?>
							<div class="alert alert-error alert-danger">
								<?php echo Text::sprintf('MOD_RSCONTACT_INPUT_ERROR', Text::_('MOD_RSCONTACT_SUBJECT')); ?>
							</div>
						<?php }
						break;
				} ?>
					</div>
				</div>
	<?php } ?>

	<?php require ModuleHelper::getLayoutPath('mod_rscontact', 'default_custom'); ?>

		<?php if ($show_message) {
				if ($message_req) {
					$class = 'required'.$class_auto;
				} else {
					$class = 'ignore'.$class_auto;
				}
				$placeholder = !$show_label ? 'placeholder="'.Text::_('MOD_RSCONTACT_MESSAGE').'"' : '';
				if($message_req && !$show_label){
					$placeholder = 'placeholder="'.Text::_('MOD_RSCONTACT_MESSAGE').' '.$required_marker.'"';
				}
			?>
		<div class="control-group">
		<?php if ($show_label) {  ?>
				<label for="mod-rscontact-message-<?php echo $uniqid; ?>" class="control-label">
					<?php echo Text::_('MOD_RSCONTACT_MESSAGE');
						if ($message_req) { ?>
							<span class="help-inline"><?php echo $required_marker;?></span>
					<?php } ?>
				</label>
		<?php } ?>
			<div class="controls">
				<textarea name="mod_rscontact_message" id="mod-rscontact-message-<?php echo $uniqid; ?>" rows="5" class="<?php echo $class;?>" <?php echo $placeholder;?> maxlength="<?php echo $msg_len; ?>"></textarea>
			</div>
		</div>
		<?php } ?>

	<?php if ($show_send_copy_to_self) { ?>
			<div class="control-group">
				<div class="controls">
					<label for="mod-rscontact-selfcopy-<?php echo $uniqid; ?>" class="checkbox">
						<input type="checkbox" name="mod_rscontact_selfcopy" id="mod-rscontact-selfcopy-<?php echo $uniqid; ?>" value="1" >
						<?php echo Text::_('MOD_RSCONTACT_SEND_COPY_FRONT');?>
					</label>
				</div>
			</div>
	<?php } ?>

    <?php if ($show_display_consent) { ?>
			<div class="control-group">
				<div class="controls">
					<label for="mod-rscontact-display-consent-<?php echo $uniqid; ?>" class="checkbox">
						<input type="checkbox" name="mod_rscontact_display_consent" id="mod-rscontact-display-consent-<?php echo $uniqid; ?>" value="1" >
						<?php echo Text::_('MOD_RSCONTACT_DISPLAY_CONSENT_FRONT');?>
					</label>
				</div>
			</div>
	<?php } ?>

	<?php if ($show_captcha) { ?>
			<div class="control-group">
				<div class="controls">
			<?php if (!$rez = modRSContactHelper::captchaGenerate('onDisplay', '', $uniqid)) { ?>
					<div class="alert alert-error alert-danger">
						<?php echo Text::_('MOD_RSCONTACT_CAPTCHA_ERROR_SETUP'); ?>
					</div>
			<?php } else {
				     echo $rez;
				    } ?>
				</div>
			</div>
	<?php }  ?>

			<div class="control-group" id="mod-rscontact-error-msg-<?php echo $uniqid;?>" style="display:none"></div>

			<div class="controls">
        <button type="submit" id="mod-rscontact-submit-btn-<?php echo $uniqid;?>" name="mod_rscontact_submit-btn-<?php echo $uniqid;?>" class="btn btn-primary submit">
          <span class="icon icon-envelope"></span> <?php echo Text::_('MOD_RSCONTACT_SUBMIT_LABEL');?>
        </button>
			</div>

		<?php if ($form_post_text) { ?>
				<div class="mod-rscontact-post-text">
					<?php echo $form_post_text; ?>
				</div>
		<?php } ?>
		</fieldset>
    </form>
	<div id="mod-rscontact-msg-<?php echo $uniqid;?>" style="display:none"></div>
	<div id="mod-rscontact-warning-msg-<?php echo $uniqid;?>" style="display:none"></div>
</div>
