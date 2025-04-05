# 0404 謝睿哲

## TODO
1. head 和 body 的模型要處理一下  
2. 我在 imgui 新增的 textbox 不能用，只有 +/- 符號可以用，可能是因為鍵盤輸入被 MainScene 搶走了，看要修還是要換  
3. 我把 reset model pos 放在 `MainScene::UpdateModel` 裡面，在站立/躺下/趴下時動作，看有沒有需要換地方(沒有獨立function)

## MODIFY
1. **MainScene**  
   1. +enum Body, Action  
   2. +float alphas[PARTSNUM], betas[PARTSNUM], gammas[PARTSNUM]  
      （紀錄每個身體部位的角度）  
   3. +isActionChange （切換躺下/站立）  
   4. +MainScene::SetRotate() （記錄在 imgui 改變的角度）  
   5. +MainScene::bodyRotateMatrix() （簡化 `updateModel`）  
   6. LoadModel 把 model 改成 10 個部分  
2. **ControlWindow**  
   1. +新增 editor 視窗
   2. 新增動作lying face up/down 測試用

程式碼髒，隨意改就好