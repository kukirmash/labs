<?php
/**
* @package RSContact!
* @copyright (C) 2015 www.rsjoomla.com
* @license GPL, http://www.gnu.org/licenses/gpl-3.0.html
*/

defined('_JEXEC') or die('Restricted access');

class mod_rscontactInstallerScript
{
	public function preflight($type, $parent)
	{
		if ($type !== 'uninstall')
		{
			try
			{
				$minJoomla = '3.10.0';

				if (!class_exists('\\Joomla\\CMS\\Version'))
				{
					throw new Exception(sprintf('Please upgrade to at least Joomla! %s before continuing!', $minJoomla));
				}

				$jversion = new \Joomla\CMS\Version;
				if (!$jversion->isCompatible($minJoomla))
				{
					throw new Exception(sprintf('Please upgrade to at least Joomla! %s before continuing!', $minJoomla));
				}
			}
			catch (Exception $e)
			{
				if (class_exists('\Joomla\CMS\Factory'))
				{
					$app = \Joomla\CMS\Factory::getApplication();
				}
                elseif (class_exists('JFactory'))
				{
					$app = JFactory::getApplication();
				}

				if (!empty($app))
				{
					$app->enqueueMessage($e->getMessage(), 'error');
				}
				return false;
			}
		}

		return true;
	}

	public function postflight($type, $parent)
    {
		if ($type == 'uninstall')
        {
			return true;
		}
		
		$db = \Joomla\CMS\Factory::getDbo();
		$query = $db->getQuery(true);
		$query->select('id')
			  ->from($db->qn('#__modules'))
			  ->where($db->qn('module').' = '.$db->q('mod_rscontact'));
		$moduleId = $db->setQuery($query)->loadResult();
		?>
		<style type="text/css">
		.version-history {
			margin: 0 0 2em 0;
			padding: 0;
			list-style-type: none;
		}
		.version-history > li {
			margin: 0 0 0.5em 0;
			padding: 0 0 0 4em;
			text-align:left;
			font-weight:normal;
		}
		.version-new,
		.version-fixed,
		.version-upgraded {
			float: left;
			font-size: 0.8em;
			margin-left: -4.9em;
			width: 4.5em;
			color: white;
			text-align: center;
			font-weight: bold;
			text-transform: uppercase;
			border-radius: 4px;
		}

		.version-new {
			background: #7dc35b;
		}
		.version-fixed {
			background: #e9a130;
		}
		.version-upgraded {
			background: #61b3de;
		}

		.install-ok {
			background: #7dc35b;
			color: #fff;
			padding: 3px;
		}

		.install-not-ok {
			background: #E9452F;
			color: #fff;
			padding: 3px;
		}

		#installer-left {
			border: 1px solid #e0e0e0;
			float: left;
			margin: 10px;
		}

		#installer-right {
			float: left;
		}
		</style>

		<div id="installer-left">
			<?php echo \Joomla\CMS\HTML\HTMLHelper::_('image', 'mod_rscontact/logo.png', 'RSContact! Logo', null, true); ?>
		</div>
		<div id="installer-right">
			<h3>RSContact! v2.0.3 Changelog</h3>
			<ul class="version-history">
                <li><span class="version-new">New</span> Joomla! 5 native compatibility - no longer needs the 'Behaviour - Backward Compatibility' plugin.</li>
			</ul>
			<?php if ($moduleId) { ?>
			<a class="btn btn-primary btn-large text-white" href="index.php?option=com_modules&amp;task=module.edit&amp;id=<?php echo (int) $moduleId; ?>">Start using RSContact!</a>
			<?php } ?>
			<a class="btn btn-secondary text-white" href="https://www.rsjoomla.com/support/documentation/rscontact.html" target="_blank">Read the RSContact! User Guide</a>
			<a class="btn btn-secondary text-white" href="https://www.rsjoomla.com/forum/rscontact.html" target="_blank">Get Support!</a>
		</div>
		<div style="clear: both;"></div>
		<?php
	}
}