<?php
/**
 * @package     Joomla.Site
 * @subpackage  Templates.Strict_color
 * 
 * @copyright   (C) 2017 Open Source Matters, Inc. <https://www.joomla.org>
 * @license     GNU General Public License version 2 or later; see LICENSE.txt
 */
 
// No direct access.
defined('_JEXEC') or die;

use Joomla\CMS\Factory;

// Get params
$logo		= $this->params->get('logo');
$color      = $this->params->get('templatecolor');

// Template path & bootstrap on Joomla's site path
$template_path  = $this->baseurl . '/templates/' . $this->template;
$bootstrap_path = $this->baseurl . '/media/vendor/bootstrap/css';
$bootstrap_js_path = $this->baseurl . '/media/vendor/bootstrap/js';

// Site name for footer
$app = Factory::getApplication();
$sitename = htmlentities($app->get('sitename'), ENT_QUOTES, 'UTF-8');
?>

<!DOCTYPE html>
<html lang="<?php echo $this->language; ?>" 
      dir ="<?php echo $this->direction; ?>">

    <head>
	  <!-- jdoc:include type="head" / -->
      <jdoc:include type="styles" /> 
      <jdoc:include type="metas" />
	  <jdoc:include type="scripts" />
      
      <!-- Подключаем Bootstrap -->
	  <meta http-equiv="Content-Style-Type" content="text/css" />
	  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
      
      <link rel="stylesheet" href="<?php echo $bootstrap_path; ?>/bootstrap.css" type="text/css" />
	  <link rel="stylesheet" href="<?php echo $bootstrap_path; ?>/bootstrap-grid.css" type="text/css" />
  	  <link rel="stylesheet" href="<?php echo $bootstrap_path; ?>/bootstrap-reboot.css" type="text/css" />
  	   
      <!-- Включаем в html-файл своё стилевое оформление -->
	  <link rel="stylesheet" href="<?php echo $template_path; ?>/css/template.css" type="text/css" />
      <link rel="stylesheet" href="<?php echo $this->baseurl.'/templates/'.$this->template.'/css/'.$color.'.css'?>" 
            type="text/css" />
      
      <!-- Подключаем подсказки bootstrap -->
      <script src="<?php echo $bootstrap_js_path; ?>/bootstrap-es5.min.js"></script>
  	  <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
	</head>

  <body>
      <div class="container bodydiv">
        <!-- ******************** ШАПКА ******************** -->
		<!-- ****** фиолетовая часть макета страницы ******* -->
        <header>
            <!-- **************** СТРОКА ПОИСК ******************* -->
			<div class="row">
			  <div class="col-8">
				  <!-- пустой отступ слева -->
			  </div>
			  <div class="col-4">
				<?php if($this->countModules('strictColor-search')) : ?>
				   <jdoc:include type="modules" name="strictColor-search" />
				<?php endif; ?>
				<?php if($this->countModules('topbar')) : ?>
				   <jdoc:include type="modules" name="topbar" />
				<?php endif; ?>
			  </div>
			</div>
          
            <!-- **************** ЛОГО *********************** -->
		    <?php if($logo) : ?>
		    <div class="row">
              <div class="col-2">
                  <!-- пустой отступ от левого края для визуального эффекта -->
              </div>
			  <div class="col-9">
                 <!-- цепляем к лого подсказку bootstrap -->
			     <button type="button" class="btn btn-secondary" data-bs-toggle="tooltip" data-bs-placement="bottom" 
				        title="<?php echo 'Ура! Кофе!'; ?>">
      		        <img class="logo" 
				         src="<?php echo htmlentities($logo, ENT_QUOTES, 'UTF-8')?>" 
				         alt='<?php "'$sitename'"?>' /> 
			    </button>
                <script> $( document ).ready(function() {$('[data-bs-toggle="tooltip"]').tooltip();}); </script>
                <!--  из https://getbootstrap.ru/docs/5.1/components/tooltips/#  -->
       
			  </div>
		    </div>
		    <?php endif; ?>
        </header>
        
		
        <!-- ******************** МЕНЮ ******************** --> 
		<!-- ******* голубая часть макета страницы ******** -->
        <nav class="main-navigation container">
		<!-- в первую очередь отображаем меню, привязанное к позитии нашего шаблона... -->
        <?php if($this->countModules('strictColor-topmenu')) : ?>
			<div class="row">
				<div class="col-12 topmenu">
				  <jdoc:include type="modules" name="strictColor-topmenu"/>
				</div>
			</div>
			<!-- ... потом проверяем, что отображалось в шаблоне Cassiopea -->
		<?php else: if ($this->countModules('main-top')) : ?>
			<div class="row">
				<div class="col-12 topmenu">
				  <jdoc:include type="modules" name="main-top"/>
				</div>
			</div>
			<?php endif; ?>
        <?php endif; ?>
        </nav>

              
        <!-- ************ ОСНОВНАЯ ЧАСТЬ *************** -->
		<!-- ***** оранжевая часть макета страницы ***** -->
        <main>
			<div class="row">
			  <div class="col-md-12">
				<?php if($this->countModules('strictColor-left')) : ?>
				  <!-- заполняем
					   левый оранжевый блок макета страницы -->
				  <div id="modules" class="col-md-3"> 
					<jdoc:include type="modules" name="strictColor-left" />
					<jdoc:include type="modules" name="main-top" />
					<jdoc:include type="modules" name="main-bottom" />
				  </div>
				  <!-- и средний оранжевый блок макета страницы ... -->
				  <div id="textcontent" class="col-md-6"> <!-- средний оранжевый блок макета страницы-->
					<jdoc:include type="component" />
				  </div>
				<?php  else: ?>
				  <!-- ... либо объединеняем левый и средний оранжевые
					   блоки макета страницы и выводим основное содержимое сайта (статьи) -->
				  <div id="textcontent" class="col-md-9"> 
					<jdoc:include type="component" />
				  </div>
				<?php endif; ?>  
			  
				<aside>
				<!-- заполняем
					 правый оранжевый блок макета страницы -->
				<div id="rightatab" class="col-md-3">
				  <?php if($this->countModules('strictColor-right')) : ?>
						<jdoc:include type="modules" name="strictColor-right" />
				  <?php endif; ?>
				  <?php if($this->countModules('sidebar-right')) : ?>
						<jdoc:include type="modules" name="sidebar-right" />
				  <?php endif; ?>
				</div>
				</aside>
			  </div>
			</div>
        </main>
       
            
		<!-- ****************** ПОДВАЛ ***************** --> 
		<!-- ******* серая часть макета страницы ******* -->
		<footer>
			<div class="row" id="footer">
			  <!-- заполняем
				   левый серый блок макета страницы -->
			  <div class="col-md-4">
				<?php if($this->countModules('strictColor-footer')) : ?>
					  <jdoc:include type="modules" name="strictColor-footer" />
				<?php endif; ?>
			  </div>
			  <!-- заполняем
				   центральный серый блок макета страницы -->
			  <div class="col-md-4">
				<button type="button" class="btn btn-secondary" data-bs-toggle="tooltip" data-bs-placement="bottom" 
				        title="Ещё одна подсказка">
                  <h5> Какая-то дополнительная информация на 
                     <?php echo $sitename."." ?>
                </h5>
                </button>
			  </div>
			  <!-- заполняем
				   правый серый блок макета страницы -->
			  <div class="col-md-4">
				<h5> Сообщения системы </h5>
				<jdoc:include type="message" />
				<?php if($this->countModules('debug')) : ?>
					  <jdoc:include type="modules" name="debug" />
				<?php endif; ?>
			  </div>
			</div>
		</footer>
      </div>  
	</body>
</html>
