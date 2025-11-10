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

// Check to ensure this file is included in Joomla!
defined('_JEXEC') or die( 'Restricted access' );

use Joomla\CMS\Factory;
use Joomla\CMS\Toolbar\ToolbarHelper;
use Joomla\CMS\Language\Text;
use Joomla\CMS\MVC\View\HtmlView;
use Joomla\CMS\HTML\HTMLHelper;
use Joomla\CMS\Uri\Uri;

jimport('joomla.application.component.view');

class DJImageSliderViewItems extends HtmlView
{
	protected $items;
	protected $pagination;
	protected $state;
	
	public function display($tpl = null)
	{
		$this->items		= $this->get('Items');
		$this->pagination	= $this->get('Pagination');
		$this->state		= $this->get('State');
        $this->filterForm = $this->get('FilterForm');
        $this->activeFilters = $this->get('ActiveFilters');


        // Check for errors.
		if (count($errors = $this->get('Errors'))) {
			throw new \Exception(implode("\n", $errors), 500);
			return false;
		}

		if (class_exists('JHtmlSidebar')){
			$this->sidebar = JHtmlSidebar::render();
		}
		
		foreach($this->items as $item) {
			$item->thumb = 'components/com_djimageslider/assets/icon-image.png';						
			if(strcasecmp(substr($item->image, 0, 4), 'http') != 0 && !empty($item->image)) {
				$item->image = Uri::root(true).'/'.$item->image;
			}
			$item->preview = '<img src="'.$item->image.'" alt="'.$this->escape($item->title).'" width="300" />';
		}
		
		$this->classes = DJImageSliderHelper::getBSClasses();
		
		$this->addToolbar();		
		parent::display($tpl);
	}
	
	protected function addToolbar()
	{
		$doc = Factory::getDocument();

		HTMLHelper::_('jquery.framework');
		$doc->addStyleSheet(Uri::root(true).'/media/djextensions/magnific/magnific.css');
		$doc->addScript(Uri::root(true).'/media/djextensions/magnific/magnific.js', 'text/javascript');
		$doc->addScript(Uri::base(true).'/components/com_djimageslider/assets/magnific-init.js', 'text/javascript');
		
		ToolBarHelper::title(Text::_('COM_DJIMAGESLIDER_SLIDES'), 'generic.png');

		ToolBarHelper::addNew('item.add','JTOOLBAR_NEW');
		ToolBarHelper::editList('item.edit','JTOOLBAR_EDIT');
		ToolBarHelper::deleteList('', 'items.delete','JTOOLBAR_DELETE');
		ToolBarHelper::divider();
		ToolBarHelper::custom('items.publish', 'publish.png', 'publish_f2.png','JTOOLBAR_PUBLISH', true);
		ToolBarHelper::custom('items.unpublish', 'unpublish.png', 'unpublish_f2.png', 'JTOOLBAR_UNPUBLISH', true);
		ToolBarHelper::divider();
		ToolBarHelper::preferences('com_djimageslider', 550, 875);
		
	}
}