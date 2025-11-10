<?php
/**
 * @version $Id$
 * @package DJ-ImageSlider
 * @subpackage DJ-ImageSlider Component
 * @copyright Copyright (C) 2017 DJ-Extensions.com, All rights reserved.
 * @license http://www.gnu.org/licenses GNU/GPL
 * @author url: http://dj-extensions.com
 * @author email contact@dj-extensions.com
 * @developer Szymon Woronowski - szymon.woronowski@design-joomla.eu
 *
 *
 * DJ-ImageSlider is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DJ-ImageSlider is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DJ-ImageSlider. If not, see <http://www.gnu.org/licenses/>.
 *
 */

defined('_JEXEC') or die;
use Joomla\CMS\Version;
use Joomla\CMS\Toolbar\ToolbarHelper;
use Joomla\CMS\HTML\Helpers\Sidebar;
use Joomla\CMS\Language\Text;
use Joomla\CMS\Object\CMSObject;

abstract class DJImageSliderHelper
{
	
	public static function addSubmenu($vName)
	{
		if($vName=='item' || $vName=='category') return;
		$version = new Version;
		
		if (version_compare($version->getShortVersion(), '3.0.0', '<')) {
			
			JSubMenuHelper::addEntry(
				Text::_('COM_DJIMAGESLIDER_SUBMENU_CPANEL'),
				'index.php?option=com_djimageslider',
				$vName == 'cpanel'
			);
			JSubMenuHelper::addEntry(
				Text::_('COM_DJIMAGESLIDER_SUBMENU_SLIDES'),
				'index.php?option=com_djimageslider&view=items',
				$vName == 'items'
			);
			JSubMenuHelper::addEntry(
				Text::_('COM_DJIMAGESLIDER_SUBMENU_CATEGORIES'),
				'index.php?option=com_categories&extension=com_djimageslider',
				$vName == 'categories'
			);
	
			
		} else {
			
			Sidebar::addEntry(
				Text::_('COM_DJIMAGESLIDER_SUBMENU_CPANEL'),
				'index.php?option=com_djimageslider',
				$vName == 'cpanel'
			);
			Sidebar::addEntry(
				Text::_('COM_DJIMAGESLIDER_SUBMENU_SLIDES'),
				'index.php?option=com_djimageslider&view=items',
				$vName == 'items'
			);
			Sidebar::addEntry(
				Text::_('COM_DJIMAGESLIDER_SUBMENU_CATEGORIES'),
				'index.php?option=com_categories&extension=com_djimageslider',
				$vName == 'categories'
			);
		}
		
		if ($vName=='categories') {
			ToolBarHelper::title(
			Text::sprintf('COM_DJIMAGESLIDER_CATEGORIES_TITLE',Text::_('com_djimageslider')),
			'slider-categories');
		}
	}
	
	public static function getBSClasses() {
	
		$classes = new CMSObject;
	
		if(version_compare(JVERSION, '4', '>=')) { // Bootstrap 4
			$classes->set('row', 'row');
			$classes->set('col', 'col-md-');
		} else { // Boostrap 2.3.2
			$classes->set('row', 'row-fluid');
			$classes->set('col', 'span');
		}
	
		return $classes;
	}
	
}
?>