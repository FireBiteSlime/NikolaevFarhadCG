#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}





double* normal(double **figure) 
{
		double vectorX1 = figure[0][0] - figure[1][0];
		double vectorY1 = figure[0][1] - figure[1][1];
		double vectorZ1 = figure[0][2] - figure[1][2];
		double vectorX2 = figure[1][0] - figure[2][0];
		double vectorY2 = figure[1][1] - figure[2][1];
		double vectorZ2 = figure[1][2] - figure[2][2];
		double *normalArray = new double [3];

		double wrki = sqrt((vectorY1 * vectorZ2 - vectorZ1 * vectorY2) * (vectorY1 * vectorZ2 - vectorZ1 * vectorY2) + (vectorZ1 * vectorX2 - vectorX1 * vectorZ2) * 
		(vectorZ1 * vectorX2 - vectorX1 * vectorZ2) + (vectorX1 * vectorY2 - vectorY1 * vectorX2) * (vectorX1 * vectorY2 - vectorY1 * vectorX2));
		normalArray[0] = (vectorY1 * vectorZ2 - vectorZ1 * vectorY2) / wrki;
		normalArray[1] = (vectorZ1 * vectorX2 - vectorX1 * vectorZ2) / wrki;
		normalArray[2] = (vectorX1 * vectorY2 - vectorY1 * vectorX2) / wrki;
		
		return normalArray;
}


void FigureForRotated() {
	double O[] = { 6.55,-5.02, 0 };
	double O1[] = { 6.55,-5.02,4 };
	double B[] = { 4,-8,0 };
	double B1[] = { 4,-8,4 };

	double B3[] = { 3.99,-8,0 };
	double B4[] = { 3.99,-8,4 };
	double* normalForThisAdge = new double[3];

	glBegin(GL_QUAD_STRIP);

	double* quad0[] = { O,O1,B };
	normalForThisAdge = normal(quad0);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(O);
	glVertex3dv(O1);
	glVertex3dv(B);
	glVertex3dv(B1);
	glVertex3dv(B3);
	glVertex3dv(B4);
	


	glEnd();

	glBegin(GL_TRIANGLES);

	double* triangl0[] = { O,B,B3 };
	normalForThisAdge = normal(triangl0);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], -normalForThisAdge[2]);
	glVertex3dv(O);
	glVertex3dv(B);
	glVertex3dv(B3);
	

	double* triangl01[] = { O1,B1,B4 };
	normalForThisAdge = normal(triangl01);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(O1);
	glVertex3dv(B1);
	glVertex3dv(B4);
	

	glEnd();

}
void MiddleFigure()
{
	double A[] = { 9.09,-2.03,0 };
	double B[] = { 4,-8,0 };
	double C[] = { 10.47,-5.18,0 };
	double D[] = { 10.03,-9,0 };
	double E[] = { 16,-4,0 };
	double F[] = { 9.18,-4.01,0 };

	double A1[] = { 9.09,-2.03,4 };
	double B1[] = { 4,-8,4 };
	double C1[] = { 10.47,-5.18,4 };
	double D1[] = { 10.03,-9,4 };
	double E1[] = { 16,-4,4 };
	double F1[] = { 9.18,-4.01,4 };

	double O[] = { 6.55,-5.02, 0 };
	double O1[] = { 6.55,-5.02,4 };

	double *normalForThisAdge = new double [3];


	glBegin(GL_TRIANGLES);

	glColor3d(0.39, 0.63, 0.45);

	double* triangl0[] = { A,F,B };
	normalForThisAdge = normal(triangl0);
	//glNormal3d(1, 1, -1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1],  - normalForThisAdge[2]);
	glVertex3dv(A);
	glVertex3dv(F);
	glVertex3dv(B);
	
	double* triangl1[] = { F,E,C };
	normalForThisAdge = normal(triangl1);
	//glNormal3d(1, 1, -1);
	//glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(F);
	glVertex3dv(E);
	glVertex3dv(C);
	
	double* triangl2[] = { E,C,D };
	normalForThisAdge = normal(triangl2);
	//glNormal3d(1, 1, -1);
	//glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(E);
	glVertex3dv(C);
	glVertex3dv(D);
	
	double* triangl3[] = { C,F,B };
	normalForThisAdge = normal(triangl3);
	//glNormal3d(1, 1, -1);
	//glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(C);
	glVertex3dv(F);
	glVertex3dv(B);
	


	///////////////////////upper/////////////////////////
	double* triangl00[] = { A1,F1,B1 };
	normalForThisAdge = normal(triangl00);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(A1);
	glVertex3dv(F1);
	glVertex3dv(B1);
	
	double* triangl11[] = { F1,E1,C1 };
	normalForThisAdge = normal(triangl11);
	//glNormal3d(1, 1, 1);
	//glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(F1);
	glVertex3dv(E1);
	glVertex3dv(C1);
	
	double* triangl22[] = { E1,C1,D1 };
	normalForThisAdge = normal(triangl22);
	//glNormal3d(1, 1, 1);
	//glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(E1);
	glVertex3dv(C1);
	glVertex3dv(D1);
	
	double* triangl33[] = { C1,F1,B1 };
	normalForThisAdge = normal(triangl33);
	//glNormal3d(1, 1, -1);
	//glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(C1);
	glVertex3dv(F1);
	glVertex3dv(B1);
	

	glEnd();

	glBegin(GL_QUAD_STRIP);

	glColor3d(0.43, 0.65, 0.64);

	double* quad0[] = { A,A1,B };
	normalForThisAdge = normal(quad0);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(A);
	glVertex3dv(A1);
	glVertex3dv(B);
	glVertex3dv(B1);
	
	double* quad1[] = { A,A1,F };
	normalForThisAdge = normal(quad1);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(A);
	glVertex3dv(A1);
	glVertex3dv(F);
	glVertex3dv(F1);
	
	double* quad2[] = { F,F1,E };
	normalForThisAdge = normal(quad2);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(F);
	glVertex3dv(F1);
	glVertex3dv(E);
	glVertex3dv(E1);
	
	double* quad3[] = { E,E1,D };
	normalForThisAdge = normal(quad3);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(E);
	glVertex3dv(E1);
	glVertex3dv(D);
	glVertex3dv(D1);
	
	double* quad4[] = { C,C1,D };
	normalForThisAdge = normal(quad4);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glVertex3dv(C);
	glVertex3dv(C1);
	glVertex3dv(D);
	glVertex3dv(D1);
	

	glEnd();

	double* quad5[] = { B,C,C1 };
	normalForThisAdge = normal(quad5);
	//glNormal3d(1, 1, 1);
	glNormal3d(normalForThisAdge[0], normalForThisAdge[1], normalForThisAdge[2]);
	glBegin(GL_QUADS);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(C1);
	glVertex3dv(B1);
	

	glEnd();

	// выпуклость
	for (int i = 0; i < 2000; i++) {
		glPushMatrix();
		glTranslated(6.55, -5.02, 0);
		glRotated(-0.09, 0, 0, 1);
		glTranslated(-6.55, 5.02, 0);
		FigureForRotated();
	}
	glPopMatrix();

}


void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	MiddleFigure();
	
	/*glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.2, 0.7, 0.7);*/

	//glNormal3d(0, 0, type == "upper" ? 1 : -1);
	//glVertex3d(centre_x, centre_y, centre_z);

	//for (float i = 1.595; i <= PI * 2 - 2.95; i += 0.001) {
	//	double x =  10 * cos(i);
	//	double y = 10 * sin(i);

	//	//glNormal3d(0, 0, type == "upper" ? 1 : -1);
	//	//glVertex3d(centre_x + x + 7.1, centre_y + y - 3.2, centre_z);
	//	glVertex3d(x + 14.1, y + 3.81, 1);
	//}
	//glEnd();
	//glBegin(GL_QUAD_STRIP);
	//glColor3d(0.1, 0.2, 0.9);
	//glVertex3d(14, 8, 3);
	//glVertex3d(14, 8, 1);

	//for (float i = -0.27; i <= PI - 0.25; i += 0.001) {
	//	double x = 10 * cos(i);
	//	double y = 10 * sin(i);

	//	//glNormal3d(x, y, 0); // вроде норм

	//	glVertex3d(1 + x, 1 + y, 3);
	//	glVertex3d(1 + x, 1 + y, 1);
	//}
	//glEnd();
 //  //Сообщение вверху экрана
	//glBegin(GL_TRIANGLE_FAN);
	//glColor3d(0.2, 0.7, 0.7);
	//glTexCoord2d(0, 1);

	//glVertex3d(centre_x, centre_y, centre_z);

	//for (float i = -0.27; i <= PI - 0.25; i += 0.001) {
	//	double x = 10 * cos(i);
	//	double y = 10 * sin(i);

	//	//glNormal3d(0, 0, type == "upper" ? 1 : -1);
	//	//if (i == (PI - 0.52) / 2 == i) glTexCoord2d(0.5, 0);
	//	//if (PI - 0.25 == i + 0.001) glTexCoord2d(1, 1);
	//	glTexCoord2d(x / PI - 0.25, y / PI - 0.25);
	//	glVertex3d(1 + x, 1 + y, 1);
	//}

	//glEnd();

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}