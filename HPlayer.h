// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� HPLAYER_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// HPLAYER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#ifdef HPLAYER_EXPORTS
#define HPLAYER_API __declspec(dllexport)
#else
#define HPLAYER_API __declspec(dllimport)
#endif



extern HPLAYER_API int nHPlayer;

HPLAYER_API int fnHPlayer(void);


// �����Ǵ� HPlayer.dll ������
class HPLAYER_API HPlayer {
public:
	HPlayer();
	~HPlayer();

};