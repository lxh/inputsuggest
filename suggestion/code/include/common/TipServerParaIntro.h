//this file gbk   
#ifndef _TIP_SERVER_PARA_INTRO_H_
#define _TIP_SERVER_PARA_INTRO_H_

#define TIPSERVER_PARA_INTRO "\
xmlinto:\"<intro>参数介绍: \n \
  <op>操作类型 \n \
    <t0>op=dc显示操作详情 \n \
	</t0> \n \
    <t1>op=is查询模式\n \
      <example>op=is&x=x001&p=11&w=lyx</example> \n \
      <x>项目名称(数字字母组成,4位)</x> \n \
      <p>前缀</p> \n \
      <w>主体部分</w> \n \
      <qc>是否开启汉字使用拼音纠错功能，默认开启，0为关闭</qc> \n \
      <pns>前缀泛匹配长度</pns> \n \
      <ap>查询结果数不够的情况下，先进行全省查询[默认为0，非0为使用]</ap> \n \
      <xa>如果在制定的项目检索不全,扩展到全部</xa> \n \
      <ts>使用哪棵树进行检索，树的编号左移，然后与该值，如果为真，代表可以使用</ts> \n \
      <wng>如果条件符合自动使用九宫(默认关闭:1开启; 0关闭)</wng> \n \
      <mat>强制使用所有的task检索，优先级非常高</mat> \n \
      <sl>返回的结果长度要跟当前输入的长度一致(samelen)</sl> \n \
      <wm>在配置文件中设置了混合查询开关，并且该处设置为1时，开启混合查询</wm> \n \
	  <incode>输入的w的编码的编码格式，默认gbk,如果utf-8则输出utf-8--->>奇葩，在cgi里面处理的</incode> \n \
	  <outcode>输出的编码格式，默认gbk,如果utf-8则输出utf-8--->>奇葩，在cgi里面处理的</outcode> \n \
      <flag>标志筛选 \n \
        <flg>标志相等(该标志设置，其他标志失效)</flg> \n \
		<group>混合使用标志,为真符合条件 \n \
			<flgl>low</flgl> \n \
			<flgu>up</flgu> \n \
			<flga>and</flga> \n \
			<flgo>or,暂时不用</flgo> \n \
		</group> \n \
      </flag> \n \
	  <sffc>一些查询策略是否使用的开关, 按位进行计算，也就是实际值左移动 \n \
	  	<mark>这些设置必须索引支持才能使用，也就是如果没有建立相应的索引，也不能支持</mark> \n \
		<examle>想要全部支持，暂时可以设置为1023</example> \n \
	  	<values_1>支持混合搜索</values_1> \n \
		<values_2>支持汉字纠错，汉字转拼音</values_2> \n \
		<values_3>全部任务查询</values_3> \n \
	  	<values_4>汉字全拼混合查询</values_4> \n \
		<values_5>全拼简拼混合查询</values_5> \n \
	  </sffc> \n \
      <attr>属性信息 \n \
        <import>这里的标志是按照位进行计算的</import> \n \
        <b1>是否需要返回扩展信息</b1> \n \
        <b2>是否需要支持dot查询</b2> \n \
        <b4>是否需要支持混合(汉字,拼音)查询</b4> \n \
        <b8>是否需要支持拼写纠错</b8> \n \
        <b16>是否大小写敏感</b16> \n \
        <b32>前缀支持简单正则</b32> \n \
        <unuse>未使用</unuse> \n \
      </attr> \n \
      <lmt>返回的结果个数上限</lmt> \n \
      <fl>标志的下限(包含)flag low</fl> \n \
      <fu>标志的上限(包含)flag up</fu> \n \
      <fo>标志flag or为真</fo> \n \
      <fa>标志flag and为真</fa> \n \
    </t1> \n \
    <t2>op=so特殊处理模块 \n \
      <example>op=so&k=l&v=x001</example> \n \
      <k>[a:add; d:delete; u:update; l:look]可以进行的操作</k> \n \
      <v>项目名称(如果不填写,则代表全部,d,u,l可用)</v> \n \
    </t2> \n \
    <t3>op=dg调试模式 \n \
      <example>op=dg&x=x001&p=11&w=lyx&d=10&t=0</example> \n \
      <remark>查看x001的前缀为11,查询lyx(完全匹配)的树的样子,最多显示深度为d</remark> \n \
      <x>项目名称</x> \n \
      <p>前缀</p> \n \
      <w>主体部分(区分大小写)</w> \n \
      <d>显示的树的深度</d> \n \
      <t>树的类型[0:汉字; 1:拼音; 2:中间汉字; 3:中间拼音]</t> \n \
    </t3> \n \
  </op> \n \
</intro> \" "









#endif
