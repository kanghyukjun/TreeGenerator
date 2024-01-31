# 🌳 L-system 3D 나무 모델 생성기

졸업 과제로 진행했던 나무 모델 생성 프로젝트 입니다. OpenGL을 활용해 3D 모델을 생성하였습니다.

Rinthel Kwon님의 [OpenGL 강의 영상](https://www.youtube.com/@rinthel) 내용과 코드를 참고하였습니다.

[![youtube](https://github.com/kanghyukjun/TreeGenerator/blob/main/image/youtube.png)](https://youtu.be/kEAKvJKnvfA)

## ⏰ 개발 기간

- 2023.03.08 (수) ~ 2023.06.13 (화)
- 주제 선정
- 그래픽스 파이프라인 구성
- UI 구성
- 쉐이더 구성
- L-System 기반 나무 모델 생성

## 🔧 주요 기능 설명

### 메인 화면

![main](https://github.com/kanghyukjun/TreeGenerator/blob/main/image/main.png)
![gui](https://github.com/kanghyukjun/TreeGenerator/blob/main/image/gui.png)

- 나뭇가지의 회전 각도, 나무 두께, 길이 조절 가능
- 나뭇잎의 크기 조절 가능

### L-system 매개변수 조정

![L-system](https://github.com/kanghyukjun/TreeGenerator/blob/main/image/generate.gif)

- axiom : 초기값 설정
- rules : 규칙 선택 혹은 임의의 규칙 입력
- iteration : 세대, 반복 횟수

### Obj 파일 저장

![obj](https://github.com/kanghyukjun/TreeGenerator/blob/main/image/save_load.gif)

- 생성한 나무 모델의 obj 파일 저장과 불러오기 기능

![extern](https://github.com/kanghyukjun/TreeGenerator/blob/main/image/paint3D.png)
